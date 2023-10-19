#include "tls_transport_factory.h"
#include "i_tls_transport.h"

#include "core/event_channel_state.h"

#include "utils/property_utils.h"
#include "utils/property_writer.h"
#include "utils/message_sink_impl.h"
#include "utils/message_router_impl.h"
#include "utils/message_event_impl.h"
#include "utils/pointer_utils.h"
#include "utils/common_utils.h"
#include "utils/enum_utils.h"

#include "net/net_message_types.h"
#include "net/socket/socket_packet_impl.h"
#include "net/net_utils.h"

#include "tls_transport_params.h"
#include "tls_packet_impl.h"

#include "tools/ssl/ssl_manager.h"
#include "tools/ssl/ssl_manager_config.h"
#include "tools/ssl/ssl_connection_config.h"
#include "tools/ssl/custom_ssl_connection_observer.h"
#include "tools/ssl/const_ssl_message.h"

#include <shared_mutex>
#include <iostream>

namespace mpl::net
{

namespace detail
{


static ssl::ssl_manager_config_t create_ssl_config(const tls_config_t& tls_config)
{
    ssl::ssl_context_config_t ssl_context_params(tls_config.method == tls_method_t::dtls ? ssl::ssl_method_t::dtls_method : ssl::ssl_method_t::tls_method
                                                        , ssl::ssl_options_flags_t::default_options
                                                        , ssl::ssl_session_cache_flags_t::default_flags
                                                        , ssl::ssl_verify_flags_t::default_flags);

    ssl::ssl_manager_config_t ssl_config(ssl_context_params
                                         , {}
                                         , {}
                                         , std::string("mpl").append(utils::random_string(10))
                                         , tls_config.chipers);

    if (tls_config.srtp_enable)
    {
        ssl_config.srtp_profiles = ssl::ssl_manager_config_t::default_srtp_profiles;
    }

    return ssl_config;
}

}

class tls_transport_impl : public i_tls_transport
        , private ssl::i_ssl_connection_observer
{
    tls_config_t                        m_tls_config;
    i_timer_manager&                    m_timer_manager;
    ssl::ssl_manager&                   m_ssl_manager;
    mutable tls_transport_params_t      m_tls_params;

    ssl::i_ssl_connection::s_ptr_t      m_ssl_connection;

    message_sink_impl                   m_message_sink;
    message_router_impl                 m_router;
    i_timer::u_ptr_t                    m_timer;

    std::size_t                         m_reconnections;
    bool                                m_open;
    bool                                m_connect;
    channel_state_t                     m_state;

public:

    using u_ptr_t = std::unique_ptr<tls_transport_impl>;

    static u_ptr_t create(const tls_config_t& tls_config
                          , i_timer_manager &timer_manager
                          , ssl::ssl_manager& ssl_manager
                          , tls_transport_params_t&& tls_params)
    {
        return nullptr;
    }

    tls_transport_impl(const tls_config_t& tls_config
                       , i_timer_manager& timer_manager
                       , ssl::ssl_manager& ssl_manager
                       , tls_transport_params_t&& tls_params)
        : m_tls_config(tls_config)
        , m_timer_manager(timer_manager)
        , m_ssl_manager(ssl_manager)
        , m_tls_params(tls_params)
        , m_message_sink([&](auto&& args) { return on_send_message(args); })
        , m_timer(m_timer_manager.create_timer([&]{ on_timer(); }))
        , m_reconnections(0)
        , m_open(false)
        , m_connect(false)
        , m_state(channel_state_t::ready)
    {

    }

    ~tls_transport_impl()
    {

    }

    inline void change_channel_state(channel_state_t new_state
                              , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl(event_channel_state_t(new_state
                                                                           , reason))
                                  );
        }

    }

    inline ssl::ssl_connection_config_t create_ssl_connection_config() const
    {
        return
        {
            static_cast<ssl::ssl_role_t>(m_tls_params.role)
            , static_cast<std::uint32_t>(m_tls_config.mtu_size == 0
                                         ? tls_config_t::default_mtu_size
                                         : m_tls_config.mtu_size)
        };
    }


    inline ssl::i_ssl_connection::s_ptr_t create_ssl_connection()
    {
        return m_ssl_manager.create_connection(create_ssl_connection_config()
                                               , nullptr);
    }

    void set_ssl_connection(ssl::i_ssl_connection::s_ptr_t&& ssl_connection)
    {
        if (m_ssl_connection != ssl_connection)
        {
            if (m_ssl_connection != nullptr)
            {
                m_ssl_connection->set_observer(nullptr);
            }

            m_ssl_connection = std::move(ssl_connection);

            if (m_ssl_connection != nullptr)
            {
                m_ssl_connection->set_observer(this);
            }

            update_local_params();
        }
    }

    inline bool update_local_params()
    {
        m_tls_params.local_endpoint.fingerprint.hash.clear();

        if (m_ssl_connection != nullptr)
        {
            if (m_tls_params.local_endpoint.fingerprint.is_defined())
            {
                m_tls_params.local_endpoint.fingerprint.hash = get_fingerprint(true
                                                                , m_tls_params.local_endpoint.fingerprint.method);
            }

            return m_ssl_connection->set_config(create_ssl_connection_config());
            // m_ssl_connection
        }

        return false;
    }

    inline tls_fingerprint_t::hash_t get_fingerprint(bool local
                                                     , tls_hash_method_t method) const
    {
        if (m_ssl_connection != nullptr)
        {
            return m_ssl_connection->get_fingerprint(local
                                                     ? ssl::fingerprint_direction_t::self
                                                     : ssl::fingerprint_direction_t::peer
                                                     , static_cast<ssl::hash_method_t>(method));
        }
        return {};
    }

    inline bool check_fingerprint(bool local
                                  , const tls_fingerprint_t& fingerprint) const
    {
        return !fingerprint.is_defined()
                || (get_fingerprint(local
                                   , fingerprint.method) == fingerprint.hash);
    }


    bool open()
    {
        if (!m_open)
        {
            m_reconnections = 0;
            change_channel_state(channel_state_t::opening);
            m_open = true;

            if (auto ssl_connection = create_ssl_connection())
            {
                set_ssl_connection(std::move(ssl_connection));
                change_channel_state(channel_state_t::open);
                return true;
            }

            m_open = false;
            change_channel_state(channel_state_t::failed);
        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            m_open = false;
            timer_stop();
            if (m_ssl_connection != nullptr)
            {
                if (m_ssl_connection->state() == ssl::ssl_handshake_state_t::done)
                {
                    change_channel_state(channel_state_t::closing);
                }

                m_ssl_connection->control(ssl::ssl_control_id_t::shutdown);
                set_ssl_connection(nullptr);
            }
            change_channel_state(channel_state_t::closed);
            return true;
        }

        return false;
    }

    bool connect()
    {
        if (m_open
                && !m_connect)
        {
            m_connect = true;
            if (m_ssl_connection->control(ssl::ssl_control_id_t::handshake))
            {
                return true;
            }
            m_connect = false;
            change_channel_state(channel_state_t::failed);
        }

        return false;
    }

    bool shutdown()
    {
        if (m_open
                && m_connect)
        {
            timer_stop();
            m_ssl_connection->control(ssl::ssl_control_id_t::shutdown);
            m_connect = false;
        }

        return false;
    }


    bool on_message_packet(const i_message_packet& packet)
    {
        if (packet.subclass() == message_class_net)
        {
            auto& net_packet = static_cast<const i_net_packet&>(packet);
            switch(net_packet.transport_id())
            {
                case transport_id_t::tls:

                break;
                default:;
            }


        }
        return false;
    }

    bool on_send_message(const i_message& message)
    {
        // shared_lock_t lock(m_safe_mutex);
        switch(message.category())
        {
            case message_category_t::packet:
                return on_message_packet(static_cast<const i_message_packet&>(message));
            break;
        }

        return false;
    }

    void timer_start(timestamp_t timeout = timestamp_null)
    {
        m_timer->start(timeout);
    }

    void timer_stop()
    {
        m_timer->stop();
    }

    void on_timer()
    {

    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:
                return open();
            break;
            case channel_control_id_t::close:
                return close();
            break;
            case channel_control_id_t::connect:
                return connect();
            break;
            case channel_control_id_t::shutdown:
                return shutdown();
            break;
            default:;
        }

        return false;
    }

    bool is_open() const override
    {
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        return false;
    }

    bool get_params(i_property &params) const override
    {
        return false;
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_message_sink;
        }

        return nullptr;
    }

    i_message_source *source(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_router;
        }

        return nullptr;
    }

    // i_transport_channel interface
public:
    transport_id_t transport_id() const override
    {
        return transport_id_t::tls;
    }

    // i_tls_transport interface
public:
    role_t role() const override
    {
        return m_tls_params.role;
    }

    tls_method_t method() const override
    {
        return m_tls_config.method;
    }

    tls_endpoint_t get_local_endpoint() const override
    {
        return m_tls_params.local_endpoint;
    }

    tls_endpoint_t get_remote_endpoint() const override
    {
        return m_tls_params.remote_endpoint;
    }

    bool set_local_endpoint(const tls_endpoint_t &endpoint) override
    {
        m_tls_params.local_endpoint.fingerprint.method = endpoint.fingerprint.method;
        return true;
    }

    bool set_remote_endpoint(const tls_endpoint_t &endpoint) override
    {
        m_tls_params.remote_endpoint = endpoint;
        return true;
    }

    // i_ssl_connection_observer interface
public:
    void on_message(const ssl::i_ssl_message &message) override
    {
        smart_buffer buffer(message.data()
                           , message.size());

        switch(message.type())
        {
            case ssl::ssl_data_type_t::application:
                // m_senders.send_message(const_raw_packet(std::move(buffer)));
            break;
            case ssl::ssl_data_type_t::encrypted:
                // m_senders.send_message(const_dtls_packet(std::move(buffer)));
            break;
        }
    }

    void on_state(ssl::ssl_handshake_state_t state) override
    {
        switch(state)
        {
            case ssl::ssl_handshake_state_t::ready:
                change_channel_state(channel_state_t::closed);
            break;
            case ssl::ssl_handshake_state_t::prepare:
                //change_state(channel_state_t::connecting);
            break;
            case ssl::ssl_handshake_state_t::handshaking:
                change_channel_state(channel_state_t::connecting);
            break;
            case ssl::ssl_handshake_state_t::done:
            {
                timer_stop();
                change_channel_state(channel_state_t::connected);
            }
            break;
            case ssl::ssl_handshake_state_t::closed:
                change_channel_state(channel_state_t::closed);
            break;
            case ssl::ssl_handshake_state_t::failed:
                timer_stop();
                change_channel_state(channel_state_t::failed);
            break;
        }
    }

    void on_srtp_key_info(const ssl::srtp_key_info_t &client_key
                          , const ssl::srtp_key_info_t &server_key) override
    {
        switch (m_ssl_connection->config().role)
        {
            case ssl::ssl_role_t::client:
                // m_event_muxer.send_srtp_info(server_key
                                             // , client_key);
            break;
            case ssl::ssl_role_t::server:
                // m_event_muxer.send_srtp_info(client_key, server_key);
            break;
            default:;
        }
    }

    void on_wait_timeout(uint64_t timeout) override
    {
        if (m_connect)
        {
            if (timeout > 0)
            {
                timer_start(durations::microseconds(timeout + 1));
            }
        }
    }

    bool on_verify(int32_t ok) override
    {
        if (!ok)
        {
            return check_fingerprint(false
                                     , m_tls_params.remote_endpoint.fingerprint);
        }
        return true;
    }

    void on_error(ssl::ssl_alert_type_t alert_type
                  , const std::string &reason) override
    {
        // ???
    }
};

class tls_transport_factory::pimpl_t
{
    tls_config_t        m_tls_config;
    i_timer_manager&    m_timer_manager;

    ssl::ssl_manager    m_ssl_manager;

public:
    using u_ptr_t = std::unique_ptr<pimpl_t>;

    static u_ptr_t create(const tls_config_t& tls_config
                          , i_timer_manager& timer_manager)
    {
        return std::make_unique<pimpl_t>(tls_config
                                         , timer_manager);
    }

    pimpl_t(const tls_config_t& tls_config
            , i_timer_manager& timer_manager)
        : m_tls_config(tls_config)
        , m_timer_manager(timer_manager)
        , m_ssl_manager(detail::create_ssl_config(tls_config))
    {

    }

    tls_transport_impl::u_ptr_t create_transport(const i_property &params)
    {
        tls_transport_params_t tls_params;
        if (utils::property::deserialize(tls_params
                                         , params)
                && tls_params.is_valid())
        {

        }

        return nullptr;
    }

};

tls_transport_factory::u_ptr_t tls_transport_factory::create(const tls_config_t& tls_config
                                                             , i_timer_manager &timer_manager)
{
    return std::make_unique<tls_transport_factory>(tls_config
                                                   , timer_manager);
}

tls_transport_factory::tls_transport_factory(const tls_config_t& tls_config
                                             , i_timer_manager &timer_manager)
    : m_pimpl(pimpl_t::create(tls_config
                              , timer_manager))
{

}

tls_transport_factory::~tls_transport_factory()
{

}

i_transport_channel::u_ptr_t tls_transport_factory::create_transport(const i_property &params)
{
    return m_pimpl->create_transport(params);
}

}
