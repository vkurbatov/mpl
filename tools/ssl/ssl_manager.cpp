#include "ssl_manager.h"
#include "ssl_manager_config.h"
#include "ssl_context.h"
#include "bio_context.h"
#include "ssl_adapter.h"
#include "ssl_x509.h"
#include "ssl_private_key.h"
#include "ssl_ec_key.h"
#include "ssl_connection_config.h"
#include "mapped_dtls_header.h"
#include "const_ssl_message.h"
#include "srtp_key_info.h"

#include <openssl/ssl.h>
#include <cstring>
#include <mutex>

// temporary:

#include <iostream>

namespace ssl
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

struct ssl_manager::context_t : std::enable_shared_from_this<ssl_manager::context_t>
{
    using mutex_t = std::mutex;
    using lock_t = std::lock_guard<mutex_t>;

    struct ssl_connection_t : public i_ssl_connection
    {
        using s_ptr_t = std::shared_ptr<ssl_connection_t>;

        ssl_connection_config_t                 m_config;
        i_ssl_connection_observer*              m_observer;

        ssl_manager::context_ptr_t              m_manager;
        bio_context                             m_bio_input;
        bio_context                             m_bio_output;
        ssl_adapter                             m_ssl_adapter;

        ssl_handshake_state_t                   m_state;

        static s_ptr_t create(ssl_manager::context_ptr_t&& manager
                                , const ssl_connection_config_t& config
                                , i_ssl_connection_observer* observer)
        {
            if (auto connection = std::make_shared<ssl_connection_t>(std::move(manager)
                                                                     , config
                                                                     , observer))
            {
                if (connection->is_valid())
                {
                    return connection;
                }
            }

            return nullptr;
        }

        ~ssl_connection_t()
        {
            internal_reset(ssl_handshake_state_t::closed);
        }

        ssl_connection_t(ssl_manager::context_ptr_t&& manager
                         , const ssl_connection_config_t& config
                         , i_ssl_connection_observer* observer)
            : m_config(config)
            , m_observer(observer)
            , m_manager(std::move(manager))
            , m_ssl_adapter(m_manager->m_ssl_context.native_handle()
                            , m_bio_input.native_handle()
                            , m_bio_output.native_handle()
                            , [&](std::int32_t type, std::int32_t value) { on_native_ssl_state(type, value); }
                            , [&](std::int32_t ok) { return on_native_verify(ok); })
            , m_state(ssl_handshake_state_t::ready)
        {
            if (m_config.mtu > 0)
            {
                m_ssl_adapter.set_mtu(m_config.mtu);
            }
        }

        inline void change_state(ssl_handshake_state_t new_state)
        {
            if (m_state != new_state)
            {
                m_state = new_state;
                if (m_observer)
                {
                    m_observer->on_state(new_state);
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

        inline bool on_native_verify(std::int32_t ok)
        {
            return m_observer->on_verify(ok);
        }

        inline bool is_valid() const
        {
            return m_manager != nullptr
                    && m_ssl_adapter.is_valid()
                    && m_bio_input.is_valid()
                    && m_bio_output.is_valid();
        }

        inline bool is_ready() const
        {
            return m_state == ssl_handshake_state_t::ready
                    || m_state == ssl_handshake_state_t::closed;
        }

        void check_shutdown()
        {
            if (m_ssl_adapter.get_shutdown_state() != ssl_shutdown_state_t::none)
            {
                change_state(ssl_handshake_state_t::closed);
            }
        }

        void process_alert(std::int32_t error_code)
        {
            if (auto error_desc = SSL_alert_desc_string_long(error_code))
            {
                m_observer->on_error(detail::get_alert_type(error_code)
                                     , error_desc);
            }
        }

        void check_timeout()
        {
            auto timeout = m_ssl_adapter.get_timeout();
            if (timeout > 0)
            {
                m_observer->on_wait_timeout(timeout);
            }
        }

        void process_result(ssl_result_t result)
        {
            switch(result)
            {
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
                }
                break;
                default:
                    check_shutdown();
            }
        }

        std::size_t process_outgoing_data()
        {
            if (!m_bio_output.is_eof())
            {
                bio_context::bio_data_info_t data_info;
                if (m_bio_output.get_data_info(data_info))
                {
                    //std::cout << this << ": ssl outgoing data: " << data_info.size << std::endl;
                    m_observer->on_message(const_ssl_message(data_info.data
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
                    m_observer->on_message(const_ssl_message(ssl_read_buffer.data()
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

        bool client_handshake()
        {
            change_state(ssl_handshake_state_t::prepare);
            m_ssl_adapter.set_state(ssl_state_t::clear);
            m_ssl_adapter.set_state(ssl_state_t::connect);
            auto result = m_ssl_adapter.set_state(ssl_state_t::handshaking);
            process_result(result);
            process_outgoing_data();
            return true;
        }

        bool server_handshake()
        {

            m_ssl_adapter.set_state(ssl_state_t::clear);
            change_state(ssl_handshake_state_t::prepare);
            m_ssl_adapter.set_state(ssl_state_t::accept);
            // m_ssl_adapter.set_state(ssl_state_t::handshaking);
            return true;
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
                        srtp_key_info_t client_key(srtp_profile
                                                   , srtp_key_t(master_length));
                        srtp_key_info_t server_key(srtp_profile
                                                   , srtp_key_t(master_length));

                        std::memcpy(client_key.key.data()
                                    , key.data()
                                    , key_length);

                        std::memcpy(server_key.key.data()
                                    , key.data() + key_length
                                    , key_length);

                        std::memcpy(client_key.key.data() + key_length
                                    , key.data() + key_length + key_length
                                    , salt_length);

                        std::memcpy(server_key.key.data() + key_length
                                    , key.data() + key_length + key_length + salt_length
                                    , salt_length);

                        m_observer->on_srtp_key_info(client_key
                                                     , server_key);

                    }
                }
            }
        }

        ssl_io_result_t send_encription_data(const void* data, std::size_t size)
        {
            if (m_bio_input.write(data, size) == size)
            {
                process_application_data();
                process_outgoing_data();
            }
            return ssl_io_result_t::ok;
        }

        ssl_io_result_t send_application_data(const void* data, std::size_t size)
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

        bool internal_handshake()
        {
            switch(m_config.role)
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

        bool internal_shutdown()
        {
            m_ssl_adapter.set_state(ssl_state_t::shutdown);
            if (process_outgoing_data() > 0)
            {
                return true;
            }
            return false;
        }

        bool internal_retransmit()
        {
            if (m_ssl_adapter.set_state(ssl_state_t::timeout) == ssl_result_t::ok)
            {
                if (process_outgoing_data() > 0)
                {
                    check_timeout();
                    return true;
                }
            }
            return false;
        }

        void internal_reset(ssl_handshake_state_t reset_state = ssl_handshake_state_t::ready)
        {
            if (m_state != ssl_handshake_state_t::ready)
            {
                m_ssl_adapter.set_state(ssl_state_t::shutdown);
            }
            m_ssl_adapter.set_state(ssl_state_t::clear);
            change_state(reset_state);
        }

        // i_ssl_connection interface
public:
        const ssl_connection_config_t &config() const override
        {
            return m_config;
        }

        bool set_config(const ssl_connection_config_t &ssl_connection_config) override
        {
            if (is_ready())
            {
                m_ssl_adapter.set_mtu(ssl_connection_config.mtu);
                m_config = ssl_connection_config;

                return true;
            }

            return false;
        }

        ssl_handshake_state_t state() const override
        {
            return m_state;
        }

        bool control(ssl_control_id_t control_id) override
        {
            if (m_observer)
            {
                switch(control_id)
                {
                    case ssl_control_id_t::handshake:
                        return internal_handshake();
                    break;
                    case ssl_control_id_t::shutdown:
                        return internal_shutdown();
                    break;
                    case ssl_control_id_t::retransmit:
                        return internal_retransmit();
                    break;
                    case ssl_control_id_t::reset:
                        internal_reset();
                        return true;
                    break;
                }
            }

            return false;
        }

        bool set_observer(i_ssl_connection_observer* observer) override
        {
            if (is_ready())
            {
                m_observer = observer;
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
                    return send_encription_data(message.data(), message.size());
                break;
            }

            return ssl_io_result_t::not_impl;
        }
        // i_ssl_connection interface
    public:
        fingerprint_t get_fingerprint(fingerprint_direction_t direction
                                    , hash_method_t hash_method) const override
        {
            fingerprint_t fingerprint;
            std::size_t result = 0;
            if (hash_method != hash_method_t::none)
            {
                if (direction == fingerprint_direction_t::self)
                {
                    result = m_manager->m_certificate.digest(hash_method
                                                             , fingerprint);

                }
                else if (auto remote_cert = m_ssl_adapter.get_peer_certificate())
                {
                    result = ssl_x509::digest(remote_cert
                                              , hash_method
                                              , fingerprint);
                }
            }
            if (result != fingerprint.size())
            {
                fingerprint.clear();
            }

            return fingerprint;
        }
    };

    mutable mutex_t         m_safe_mutex;

    ssl_manager_config_t    m_config;

    ssl_x509                m_certificate;
    ssl_private_key         m_private_key;
    ssl_context             m_ssl_context;

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

    static ssl_manager::context_ptr_t create(const ssl_manager_config_t& config)
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
                    return std::make_shared<context_t>(config
                                                       , std::move(cert_ctx)
                                                       , std::move(pkey_ctx)
                                                       , std::move(ssl_ctx));
                }
            }
        }
        return nullptr;
    }

    context_t(const ssl_manager_config_t& config
              , x509_ptr_t&& x509_ctx
              , evp_pkey_ptr_t&&  pkey_ctx
              , ssl_ctx_ptr_t&& ssl_ctx)
        : m_config(config)
        , m_certificate(std::move(x509_ctx))
        , m_private_key(std::move(pkey_ctx))
        , m_ssl_context(std::move(ssl_ctx))
    {
        m_ssl_context.set_read_ahead(true);
        m_ssl_context.set_verify_depth(4);
        if (m_config.ciper_list.empty())
        {
            m_ssl_context.set_cipher_list(m_config.ciper_list);
        }
        m_ssl_context.set_ecdh_auto(true);
        if (config.has_srtp())
        {
            m_ssl_context.set_tlsext_use_srtp(m_config.srtp_profile_list());
        }

    }

    ~context_t()
    {

    }

    std::size_t get_fingerprint(fingerprint_t &fingerprint
                                , hash_method_t hash_method) const
    {
        lock_t lock(m_safe_mutex);
        return m_certificate.digest(hash_method
                                    , fingerprint);
    }

    i_ssl_connection::s_ptr_t create_connection(const ssl_connection_config_t& connection_config
                                                  , i_ssl_connection_observer* observer)
    {
        lock_t lock(m_safe_mutex);
        return ssl_connection_t::create(shared_from_this()
                                        , connection_config
                                        , observer);

        return nullptr;
    }


    bool is_valid() const
    {
        return m_ssl_context.is_valid()
                && m_certificate.is_valid()
                && m_private_key.is_valid();
    }

};

ssl_manager::ssl_manager(const ssl_manager_config_t& manager_config)
    : m_context(context_t::create(manager_config))
{

}

std::size_t ssl_manager::get_fingerprint(fingerprint_t &fingerprint
                                         , hash_method_t hash_method) const
{
    return m_context->get_fingerprint(fingerprint
                                      , hash_method);
}

i_ssl_connection::s_ptr_t ssl_manager::create_connection(const ssl_connection_config_t &connection_config
                                                           , i_ssl_connection_observer* observer)
{
    return m_context->create_connection(connection_config
                                        , observer);
}


bool ssl_manager::is_valid() const
{
    return m_context != nullptr
            && m_context->is_valid();
}

}
