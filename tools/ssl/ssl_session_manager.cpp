#include "ssl_session_manager.h"
#include "ssl_certificate_impl.h"
#include "ssl_manager_config.h"
#include "ssl_session_params.h"
#include "i_ssl_message_sink.h"

#include "const_ssl_message.h"
#include "ssl_private_key.h"
#include "ssl_ec_key.h"
#include "ssl_context.h"
#include "ssl_adapter.h"
#include "bio_context.h"

#include "mapped_dtls_header.h"
#include "srtp_key_info.h"

#include <openssl/ssl.h>

#include <cstring>
#include <mutex>

namespace pt::ssl
{

static std::string dtls_srtp_label = "EXTRACTOR-dtls_srtp";

namespace detail
{


ssl_alert_type_t get_alert_type(std::int32_t alert)
{
    switch(alert >> 8)
    {
        case SSL3_AL_WARNING:
            return ssl_alert_type_t::warning;
        break;
        case SSL3_AL_FATAL:
            return ssl_alert_type_t::fatal;
        break;
    }

    return ssl_alert_type_t::undefined;
}

}

constexpr const std::size_t ssl_read_buffer_size = 0xffff;

struct ssl_session_manager::pimpl_t
{
    using mutex_t = std::mutex;
    using lock_t = std::lock_guard<mutex_t>;

    class ssl_session_impl : public i_ssl_session
            , public i_ssl_message_sink
    {
        ssl_session_manager::pimpl_t&   m_owner;

        ssl_session_params_t            m_params;
        i_listener*                     m_listener;

        bio_context                     m_bio_input;
        bio_context                     m_bio_output;
        ssl_adapter                     m_ssl_adapter;

        ssl_certificate_impl            m_peer_certificate;

        ssl_handshake_state_t           m_state;

    public:



        using u_ptr_t = std::unique_ptr<ssl_session_impl>;

        static u_ptr_t create(pimpl_t& owner
                              , const ssl_session_params_t& params
                              , i_listener* listener)
        {
            if (owner.is_valid())
            {
                if (auto session = std::make_unique<ssl_session_impl>(owner
                                                                      , params
                                                                      , listener))
                {
                    if (session->is_valid())
                    {
                        return session;
                    }
                }
            }

            return nullptr;
        }

        ssl_session_impl(pimpl_t& owner
                         , const ssl_session_params_t& params
                         , i_listener* listener)
            : m_owner(owner)
            , m_params(params)
            , m_listener(listener)
            , m_ssl_adapter(m_owner.m_ssl_context.native_handle()
                            , m_bio_input.native_handle()
                            , m_bio_output.native_handle()
                            , [&](auto&& ...args) { on_native_ssl_state(args...); }
                            , [&](auto&& ...args) { return on_native_verify(args...); })
            , m_state(ssl_handshake_state_t::ready)
        {
            m_ssl_adapter.set_mtu(m_params.mtu);
        }


        inline void change_state(ssl_handshake_state_t new_state)
        {
            if (m_state != new_state)
            {
                m_state = new_state;
                if (m_listener)
                {
                    m_listener->on_state(new_state);
                }
            }
        }

        void on_native_ssl_state(std::int32_t type
                                 , std::int32_t value)
        {

            // std::cout << this << ": ssl state: type: " << type << ", v: " << value << std::endl;
            ssl_state_flags_t state_flags = static_cast<ssl_state_flags_t>(type);

            if (get_enum_flag(state_flags, ssl_state_flags_t::loop))
            {

            }
            else if (get_enum_flag(state_flags, ssl_state_flags_t::alert))
            {
                process_alert(value);
                if (detail::get_alert_type(value) == ssl_alert_type_t::warning)
                {
                    switch(value & 0xff)
                    {
                        case SSL_AD_CLOSE_NOTIFY:
                            change_state(ssl_handshake_state_t::closed);
                        break;
                    }

                }
            }
            else if (get_enum_flag(state_flags, ssl_state_flags_t::exit))
            {
                if (value == 0)
                {
                    change_state(ssl_handshake_state_t::failed);
                }
            }
            else if (get_enum_flag(state_flags, ssl_state_flags_t::handshake_start))
            {
                change_state(ssl_handshake_state_t::handshaking);
            }
            else if (get_enum_flag(state_flags, ssl_state_flags_t::handshake_done))
            {
                srtp_keys_process();
                change_state(ssl_handshake_state_t::done);
            }
        }

        inline bool on_native_verify(std::int32_t ok
                                     , x509_store_ctx_st* ctx)
        {
            if (ok == 1)
            {
                m_peer_certificate.x509().set(create_object<X509>(X509_STORE_CTX_get_current_cert(ctx)));

                if (m_peer_certificate.is_valid())
                {
                    return m_listener->on_verify_certificate(&m_peer_certificate);
                }

                return false;
            }

            return true;
        }

        inline bool is_valid() const
        {
            return m_owner.is_valid()
                    && m_ssl_adapter.is_valid()
                    && m_bio_input.is_valid()
                    && m_bio_output.is_valid();
        }

        inline bool is_ready() const
        {
            return m_state == ssl_handshake_state_t::ready
                    || m_state == ssl_handshake_state_t::closed;
        }

        inline void check_shutdown()
        {
            if (m_ssl_adapter.get_shutdown_state() != ssl_shutdown_state_t::none)
            {
                change_state(ssl_handshake_state_t::closed);
            }
        }

        inline void process_alert(std::int32_t error_code)
        {
            if (auto error_desc = SSL_alert_desc_string_long(error_code))
            {
                m_listener->on_error(detail::get_alert_type(error_code)
                                     , error_desc);
            }
        }

        inline void check_timeout()
        {
            auto timeout = m_ssl_adapter.get_timeout();
            if (timeout > 0)
            {
                m_listener->on_wait_timeout(timeout);
            }
            return;
        }

        inline bool process_result(ssl_result_t result)
        {
            switch(result)
            {
                case ssl_result_t::ok:
                case ssl_result_t::want_write:
                    return true;
                    // nothng ??
                break;
                case ssl_result_t::error_syscall:
                    change_state(ssl_handshake_state_t::failed);
                break;
                case ssl_result_t::ssl_error:
                    // no read data ?
                break;
                case ssl_result_t::handoff:
                    change_state(ssl_handshake_state_t::closed);
                break;
                case ssl_result_t::want_read:
                {
                    check_timeout();
                    return true;
                }
                break;
                default:
                    check_shutdown();
            }

            return false;
        }

        std::size_t process_outgoing_data()
        {
            if (!m_bio_output.is_eof())
            {
                bio_context::bio_data_info_t data_info;
                if (m_bio_output.get_data_info(data_info))
                {
                    //std::cout << this << ": ssl outgoing data: " << data_info.size << std::endl;
                    m_listener->on_message(const_ssl_message(data_info.data
                                                             , data_info.size
                                                             , ssl_data_type_t::encrypted));
                    m_bio_output.reset();

                    return data_info.size;
                }
            }

            return 0;
        }

        std::size_t process_application_data()
        {
            static thread_local std::vector<std::uint8_t> ssl_read_buffer(ssl_read_buffer_size);
            ssl_result_t result = ssl_result_t::ok;
            auto read_bytes = m_ssl_adapter.read(ssl_read_buffer.data()
                                                 , ssl_read_buffer_size
                                                 , result);

            if (result == ssl_result_t::ok)
            {
                if (read_bytes > 0)
                {
                    m_listener->on_message(const_ssl_message(ssl_read_buffer.data()
                                                               , read_bytes
                                                               , ssl_data_type_t::application));
                    return read_bytes;
                }
            }
            else
            {
                process_result(result);
            }

            return 0;
        }

        inline bool client_handshake()
        {
            enum class fsm_t
            {
                prepare,
                clear,
                connect,
                handshake,
                completed,
                failed
            };

            fsm_t fsm = fsm_t::prepare;
            auto result = ssl_result_t::ok;

            while(fsm < fsm_t::failed)
            {

                switch(fsm)
                {
                    case fsm_t::prepare:
                    {
                        change_state(ssl_handshake_state_t::prepare);
                    }
                    break;
                    case fsm_t::clear:
                    {
                        result = m_ssl_adapter.control(ssl_control_id_t::clear);
                    }
                    break;
                    case fsm_t::connect:
                    {
                        result = m_ssl_adapter.control(ssl_control_id_t::connect);
                    }
                    break;
                    case fsm_t::handshake:
                    {
                        result = m_ssl_adapter.control(ssl_control_id_t::handshaking);
                        process_outgoing_data();
                    }
                    break;
                    case fsm_t::completed:
                        return true;
                    break;
                    default:;
                }

                if (process_result(result))
                {
                    result = ssl_result_t::ok;
                    fsm = static_cast<fsm_t>(static_cast<std::int32_t>(fsm) + 1);
                }
                else
                {
                    fsm = fsm_t::failed;
                }

            }

            return false;
        }

        inline bool server_handshake()
        {
            enum class fsm_t
            {
                prepare,
                clear,
                accept,
                completed,
                failed
            };

            fsm_t fsm = fsm_t::prepare;
            auto result = ssl_result_t::ok;

            while(fsm < fsm_t::failed)
            {

                switch(fsm)
                {
                    case fsm_t::prepare:
                    {
                        change_state(ssl_handshake_state_t::prepare);
                    }
                    break;
                    case fsm_t::clear:
                    {
                        result = m_ssl_adapter.control(ssl_control_id_t::clear);
                    }
                    break;
                    case fsm_t::accept:
                    {
                        result = m_ssl_adapter.control(ssl_control_id_t::accept);
                    }
                    break;
                    case fsm_t::completed:
                        return true;
                    break;
                    default:;
                }

                if (process_result(result))
                {
                    result = ssl_result_t::ok;
                    fsm = static_cast<fsm_t>(static_cast<std::int32_t>(fsm) + 1);
                }
                else
                {
                    fsm = fsm_t::failed;
                }

            }

            return false;
        }

        void srtp_keys_process()
        {
            auto srtp_profile = m_ssl_adapter.get_selected_srtp_profile();
            if (srtp_profile != srtp_profile_id_t::none)
            {
                auto key_length = srtp_key_info_t::get_key_length(srtp_profile);
                auto salt_length = srtp_key_info_t::get_salt_length(srtp_profile);
                auto master_length = key_length + salt_length;

                if (master_length > 0)
                {
                    srtp_key_t key(master_length * 2);
                    if (m_ssl_adapter.export_keying_material(dtls_srtp_label
                                                             , key.data()
                                                             , key.size()))
                    {

                        srtp_key_info_t encrypted_key(srtp_profile
                                                   , srtp_key_t(master_length));

                        srtp_key_info_t decrypted_key(srtp_profile
                                                   , srtp_key_t(master_length));

                        std::memcpy(decrypted_key.key.data()
                                    , key.data()
                                    , key_length);

                        std::memcpy(encrypted_key.key.data()
                                    , key.data() + key_length
                                    , key_length);

                        std::memcpy(decrypted_key.key.data() + key_length
                                    , key.data() + key_length + key_length
                                    , salt_length);

                        std::memcpy(encrypted_key.key.data() + key_length
                                    , key.data() + key_length + key_length + salt_length
                                    , salt_length);

                        if (m_params.role == ssl_role_t::client)
                        {
                            std::swap(encrypted_key
                                      , decrypted_key);
                        }

                        m_listener->on_srtp_key_info(encrypted_key
                                                     , decrypted_key);

                    }
                }
            }
        }


        inline ssl_io_result_t send_encryption_data(const void* data, std::size_t size)
        {
            if (m_bio_input.write(data, size) == size)
            {
                process_application_data();
                process_outgoing_data();
            }
            return ssl_io_result_t::ok;
        }

        inline ssl_io_result_t send_application_data(const void* data, std::size_t size)
        {
            if (m_state == ssl_handshake_state_t::done)
            {
                ssl_result_t result = ssl_result_t::ok;
                if (m_ssl_adapter.write(data, size, result) == size
                        && result == ssl_result_t::ok)
                {
                    process_outgoing_data();
                }
            }

            return ssl_io_result_t::failed;
        }


        inline bool internal_handshake()
        {
            switch(m_params.role)
            {
                case ssl_role_t::client:
                    return client_handshake();
                break;
                case ssl_role_t::server:
                    return server_handshake();
                break;
                default:;
            }
            return false;
        }

        inline bool internal_shutdown()
        {
            m_ssl_adapter.control(ssl_control_id_t::shutdown);
            if (process_outgoing_data() > 0)
            {
                return true;
            }

            return false;
        }

        inline bool internal_retransmit()
        {
            if (m_ssl_adapter.control(ssl_control_id_t::timeout) == ssl_result_t::ok)
            {
                if (process_outgoing_data() > 0)
                {
                    check_timeout();
                    return true;
                }
            }
            return false;
        }

        inline void reset_ssl()
        {

            m_ssl_adapter.control(ssl_control_id_t::clear);
            m_peer_certificate.x509().set(nullptr);
            m_ssl_adapter.set_ssl(m_owner.m_ssl_context.native_handle()
                                  , m_bio_input.native_handle()
                                  , m_bio_output.native_handle());
            m_ssl_adapter.set_mtu(m_params.mtu);
        }

        inline void internal_reset(ssl_handshake_state_t reset_state = ssl_handshake_state_t::ready)
        {
            if (m_state != ssl_handshake_state_t::ready)
            {
                m_ssl_adapter.control(ssl_control_id_t::shutdown);
            }

            reset_ssl();
            change_state(reset_state);
        }

        // i_ssl_session interface
    public:
        const ssl_session_params_t &params() const override
        {
            return m_params;
        }

        bool set_params(const ssl_session_params_t &params) override
        {
            if (is_ready())
            {
                m_params = params;
                m_ssl_adapter.set_mtu(m_params.mtu);
                return true;
            }

            return false;
        }

        ssl_handshake_state_t state() const override
        {
            return m_state;
        }

        bool control(ssl_session_control_id_t control_id) override
        {
            if (m_listener)
            {
                switch(control_id)
                {
                    case ssl_session_control_id_t::handshake:
                        return internal_handshake();
                    break;
                    case ssl_session_control_id_t::shutdown:
                        return internal_shutdown();
                    break;
                    case ssl_session_control_id_t::retransmit:
                        return internal_retransmit();
                    break;
                    case ssl_session_control_id_t::reset:
                        internal_reset();
                        return true;
                    break;
                }
            }

            return false;
        }
        i_ssl_message_sink *sink() override
        {
            return this;
        }
        const i_ssl_certificate *local_certificate() const override
        {
            return m_owner.certificate();
        }

        const i_ssl_certificate *remote_certificate() const override
        {
            return &m_peer_certificate;
        }

        bool set_listener(i_listener *listener) override
        {
            if (is_ready())
            {
                m_listener = listener;
                return true;
            }

            return false;
        }
        // i_ssl_message_sink interface
    public:
        ssl_io_result_t send_message(const i_ssl_message &message) override
        {
            switch(message.type())
            {
                case ssl_data_type_t::application:
                    return send_application_data(message.data(), message.size());
                break;
                case ssl_data_type_t::encrypted:
                    return send_encryption_data(message.data(), message.size());
                break;
            }

            return ssl_io_result_t::not_impl;
        }
    };

    using u_ptr_t = std::unique_ptr<pimpl_t>;

    mutable mutex_t             m_safe_mutex;

    ssl_manager_config_t        m_config;

    ssl_certificate_impl        m_certificate;
    ssl_private_key             m_private_key;
    ssl_context                 m_ssl_context;

    static evp_pkey_ptr_t load_private_key(const std::string& pkey_string)
    {
        if (!pkey_string.empty())
        {
            return ssl_private_key::create_evp_pkey(pkey_string);
        }

        if (auto ec_key = ssl_ec_key::create_ec_key(NID_X9_62_prime256v1))
        {
            return ssl_private_key::create_evp_pkey(std::move(ec_key));
        }

        return nullptr;
    }

    static x509_ptr_t load_certificate(const std::string& certificate_string
                                           , const evp_pkey_ptr_t& pkey_ctx
                                           , const std::string subject)
    {
        if (!certificate_string.empty())
        {
            return ssl_x509::create_x509(certificate_string);
        }

        if (pkey_ctx != nullptr)
        {
            return ssl_x509::create_x509(pkey_ctx
                                         , subject);
        }

        return nullptr;
    }

    static u_ptr_t create(const ssl_manager_config_t &config)
    {
        if (auto pkey_ctx = load_private_key(config.private_key))
        {
            if (auto cert_ctx = load_certificate(config.certificate
                                                 , pkey_ctx
                                                 , config.subject))
            {
                if (auto ssl_ctx = ssl_context::create_ssl_ctx(config.context_params
                                                               , cert_ctx
                                                               , pkey_ctx))
                {
                    return std::make_unique<pimpl_t>(config
                                                     , std::move(cert_ctx)
                                                     , std::move(pkey_ctx)
                                                     , std::move(ssl_ctx));
                }
            }
        }
        return nullptr;
    }

    pimpl_t(const ssl_manager_config_t &config
            , x509_ptr_t&& x509_ctx
            , evp_pkey_ptr_t&& pkey_ctx
            , ssl_ctx_ptr_t&& ssl_ctx)
        : m_config(config)
        , m_certificate(std::move(x509_ctx))
        , m_private_key(std::move(pkey_ctx))
        , m_ssl_context(std::move(ssl_ctx))
    {

    }

    ~pimpl_t()
    {

    }

    const i_ssl_certificate* certificate() const
    {
        return &m_certificate;
    }

    i_ssl_session::u_ptr_t create_session(const ssl_session_params_t& params
                                          , i_ssl_session::i_listener* listener)
    {
        return ssl_session_impl::create(*this
                                        , params
                                        , listener);
    }

    bool is_valid() const
    {
        return m_ssl_context.is_valid()
                && m_certificate.is_valid()
                && m_private_key.is_valid();
    }
};


ssl_session_manager::ssl_session_manager(const ssl_manager_config_t &config)
    : m_pimpl(pimpl_t::create(config))
{


}

ssl_session_manager::~ssl_session_manager()
{

}

const i_ssl_certificate *ssl_session_manager::certificate() const
{
    return m_pimpl->certificate();
}

i_ssl_session::u_ptr_t ssl_session_manager::create_session(const ssl_session_params_t &params
                                                           , i_ssl_session::i_listener *listener)
{
    return m_pimpl->create_session(params
                                   , listener);
}

bool ssl_session_manager::is_valid() const
{
    return m_pimpl != nullptr
            && m_pimpl->is_valid();
}


}
