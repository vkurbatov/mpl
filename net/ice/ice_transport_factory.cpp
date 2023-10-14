#include "ice_transport_factory.h"
#include "i_ice_transport.h"

#include "core/i_message_command.h"
#include "core/event_channel_state.h"

#include "utils/property_utils.h"
#include "utils/property_writer.h"
#include "utils/message_sink_impl.h"
#include "utils/message_router_impl.h"
#include "utils/message_event_impl.h"
#include "utils/pointer_utils.h"
#include "utils/common_utils.h"

#include "net/net_message_types.h"
#include "net/socket/socket_packet_impl.h"

#include "ice_gathering_command.h"
#include "ice_gathering_state_event.h"
#include "ice_transport_params.h"
#include "ice_controller.h"

#include "tools/base/sync_base.h"
#include <shared_mutex>
#include <list>
#include <set>

namespace mpl::net
{

namespace detail
{

ice_candidate_t* find_candidate(ice_candidate_t::array_t& candidates
                                , const socket_address_t& socket_address
                                , transport_id_t transport_id = transport_id_t::udp)
{
    for (auto& c : candidates)
    {
        if (c.transport == transport_id
                && c.connection_address == socket_address)
        {
            return &c;
        }
    }

    return nullptr;
}

ice_transport_params_t create_ice_params(const i_property& ice_property)
{
    ice_transport_params_t ice_params;

    utils::property::deserialize(ice_params
                                 , ice_property);

    return ice_params;
}

}

class ice_transport_impl : public i_ice_transport
{
    using mutex_t = std::shared_mutex;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<ice_transport_impl>;

    static constexpr std::uint64_t ice_gathering_id = 0;
    static constexpr std::uint64_t ice_binding_id = 1;

    struct candidate_pair_t;

    struct ice_socket_t : public ice_controller::i_listener
    {
        using list_t = std::list<ice_socket_t>;

        ice_transport_impl&             m_owner;
        i_socket_transport::u_ptr_t     m_socket;

        ice_candidate_t::array_t        m_local_candidates;
        socket_address_t                m_mapped_address;

        ice_controller                  m_ice_controller;
        ice_gathering_state_t           m_gathering_state;

        ice_socket_t(ice_transport_impl& owner
                     , i_socket_transport::u_ptr_t&& socket)
            : m_owner(owner)
            , m_socket(std::move(socket))
            , m_ice_controller(*this
                               , m_owner.m_timer_manager)
            , m_gathering_state(ice_gathering_state_t::ready)
        {
            m_empl
        }

        ~ice_socket_t()
        {

        }

        inline socket_type_t socket_type() const
        {
            return m_socket->transport_id() == transport_id_t::tcp
                    ? socket_type_t::tcp
                    : socket_type_t::udp;
        }

        void change_gathering_state(ice_gathering_state_t new_state)
        {
            if (m_gathering_state != new_state)
            {
                m_gathering_state = new_state;
                m_owner.on_socket_gathering_state(*this
                                                  , new_state);
            }
        }

        bool on_mapped_address(const socket_address_t& mapped_address)
        {
            if (m_mapped_address != mapped_address)
            {
                m_mapped_address = mapped_address;
            }

            return false;
        }


        inline bool control(const channel_control_t& channel_control)
        {
            return m_socket->control(channel_control);
        }

        bool send_packet(const i_message_packet& packet
                         , const socket_address_t& address)
        {
            if (m_socket->state() == channel_state_t::connected)
            {
                socket_packet_impl socket_packet(smart_buffer(&packet)
                                                 , socket_endpoint_t(socket_type()
                                                                     , address));

                return m_socket->sink(0)->send_message(socket_packet);
            }

            return false;
        }

        // i_listener interface
    public:
        bool on_send_packet(const i_stun_packet &stun_packet) override
        {
            return m_socket->sink(0)->send_message(stun_packet);
        }

        std::string on_autorized(const ice_transaction_t &transaction) override
        {
            return m_owner.get_password();
        }

        bool on_request(ice_transaction_t &transaction) override
        {
            return m_owner.on_ice_request(*this
                                          , transaction);
        }

        bool on_response(ice_transaction_t &transaction) override
        {
            switch(transaction.tag)
            {
                case ice_gathering_id:
                {
                    if (transaction.response.is_success())
                    {
                        on_mapped_address(transaction.response.mapped_address);
                        if (m_gathering_state == ice_gathering_state_t::gathering)
                        {
                            change_gathering_state(ice_gathering_state_t::completed);
                        }
                    }
                }
                break;
                default:
                {
                    m_owner.on_ice_response(*this
                                            , transaction);
                }
            }

            return false;
        }
    };

    struct candidate_pair_t
    {
        using set_t = std::set<candidate_pair_t>;
        using set_ptr_t = std::set<candidate_pair_t*>;

        ice_transport_impl&         m_owner;
        ice_socket_t&               m_socket;
        ice_candidate_t             m_remote_candidate;

        ice_state_t                 m_state;

        candidate_pair_t(ice_socket_t& socket
                         , const ice_candidate_t& candidate)
            : m_owner(socket.m_owner)
            , m_socket(socket)
            , m_remote_candidate(candidate)
            , m_state(ice_state_t::frozen)
        {

        }

        ~candidate_pair_t()
        {
            change_state(ice_state_t::shutdown);
        }

        void on_timeout()
        {

        }

        bool send_packet(const i_message_packet& packet)
        {
            return false;
        }

        void change_state(ice_state_t new_state)
        {
            if (m_state == new_state)
            {
                m_state = new_state;
                m_owner.on_pair_state(*this
                                      , new_state);
            }
        }

        bool operator == (const candidate_pair_t& other) const
        {
            return this == &other;
        }
    };

    mutable mutex_t             m_safe_mutex;

    std::uint16_t               m_foundation_id;

    ice_config_t                m_ice_config;
    i_transport_factory&        m_socket_factory;
    i_timer_manager&            m_timer_manager;
    ice_transport_params_t      m_ice_params;

    ice_socket_t::list_t        m_sockets;

    message_sink_impl           m_message_sink;
    message_router_impl         m_router;
    i_timer::u_ptr_t            m_checking_timer;

    candidate_pair_t::set_t     m_pairs;
    candidate_pair_t*           m_active_pair;

    std::uint64_t               m_tie_breaker;
    bool                        m_controlling;

    bool                        m_open;
    bool                        m_connect;
    channel_state_t             m_state;
    ice_gathering_state_t       m_gathering_state;

public:

    static u_ptr_t create(const ice_config_t& ice_config
                           , i_transport_factory& socket_factory
                           , i_timer_manager& timer_manager
                           , const i_property& params)
    {
        ice_transport_params_t ice_params;
        if (utils::property::deserialize(ice_params
                                         , params)
                && ice_params.is_valid())
        {
            return std::make_unique<ice_transport_impl>(ice_config
                                                        , socket_factory
                                                        , timer_manager
                                                        , std::move(ice_params));
        }

        return false;
    }

    ice_transport_impl(const ice_config_t& ice_config
                       , i_transport_factory& socket_factory
                       , i_timer_manager& timer_manager
                       , ice_transport_params_t&& ice_params)
        : m_foundation_id(0)
        , m_ice_config(ice_config)
        , m_socket_factory(socket_factory)
        , m_timer_manager(timer_manager)
        , m_ice_params(std::move(ice_params))
        , m_message_sink([&](auto&& args) { return on_message_sink(args); })
        , m_checking_timer(m_timer_manager.create_timer([&]{ on_checking_timeout(); }))
        , m_tie_breaker(utils::random<std::uint64_t>())
        , m_controlling(false)
        , m_open(false)
        , m_connect(false)
        , m_state(channel_state_t::ready)
        , m_gathering_state(ice_gathering_state_t::ready)
    {

    }

    ~ice_transport_impl()
    {

    }


    bool initialize()
    {
        m_foundation_id = 0;
        m_ice_params.local_endpoint.candidates.clear();
        m_pairs.clear();
        m_sockets.clear();

        for (auto it = m_ice_params.sockets.begin()
             ; it != m_ice_params.sockets.end()
             ; )
        {
            if (auto socket = create_socket(*it))
            {
                m_sockets.emplace(*this
                                  , std::move(socket));
                it = std::next(it);
                continue;
            }

            it = m_ice_params.sockets.erase(it);
        }

        return !m_sockets.empty();
    }

    void update_pairs(const ice_candidate_t::array_t& remote_candidates)
    {
        // работать тут
        for (auto it = m_pairs.begin()
             ; it != m_pairs.end()
             ; )
        {
            for (const auto& c : remote_candidates)
            {
                if (it->m_remote_candidate.is_equal_endpoint(c))
                {
                    it = std::next(it);
                    break;
                }
            }

            it = m_pairs.erase(it);
        }

    }

    inline i_socket_transport::u_ptr_t create_socket(const socket_endpoint_t& socket_endpoint)
    {
        if (auto socket_params = utils::property::create_property(property_type_t::object))
        {
            property_writer writer(*socket_params);
            writer.set("local_endpoint", socket_endpoint);
            writer.set("options.reuse_address", true);

            if (auto socket = static_pointer_cast<i_socket_transport>(m_socket_factory.create_transport(*socket_params)))
            {
                if (socket->transport_id() == socket_endpoint.transport_id)
                {
                    return socket;
                }
            }
        }

        return nullptr;
    }

    inline ice_candidate_t create_candidate(const socket_address_t& connection_endpoint
                                            , transport_id_t transport_id = transport_id_t::udp
                                            , ice_candidate_type_t candidate_type = ice_candidate_type_t::host
                                            , const socket_address_t& relayed_endpoint = {})
    {
        return ice_candidate_t::build_candidate(++m_foundation_id
                                                , m_ice_params.component_id
                                                , connection_endpoint
                                                , transport_id
                                                , candidate_type
                                                 , relayed_endpoint);
    }

    void change_gathering_state(ice_gathering_state_t new_state
                                , const std::string_view& reason = {})
    {
        if (m_gathering_state == new_state)
        {
            m_gathering_state = new_state;
            m_router.send_message(message_event_impl(ice_gathering_state_event_t(new_state
                                                                                 , reason));
        }
    }

    void change_channel_state(channel_state_t new_state
                              , const std::string_view& reason = {})
    {
        if (m_state == new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl(event_channel_state_t(new_state
                                                                           , reason));
        }
    }

    std::size_t control_sockets(const channel_control_t& control)
    {
        std::size_t result = 0;

        for (auto& s : m_sockets)
        {
            if (s.control(control))
            {
                result++;
            }
        }

        return result;
    }

    bool open()
    {
        if (!m_open)
        {
            change_channel_state(channel_state_t::opening);
            m_open = true;

            if (control_sockets(channel_control_t::open()) > 0)
            {
                change_channel_state(channel_state_t::open);
                return true;
            }

            change_channel_state(channel_state_t::failed);
            m_open = false;
        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            change_channel_state(channel_state_t::closing);
            shutdown();
            m_open = false;
            control_sockets(channel_control_t::close());
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
            change_channel_state(channel_state_t::connecting);
            m_connect = true;
            if (control_sockets(channel_control_t::connect()) > 0)
            {
                return true;
            }
            change_channel_state(channel_state_t::failed);
            m_connect = false;
        }

        return false;
    }

    bool shutdown()
    {
        if (m_open
                && m_connect)
        {
            change_channel_state(channel_state_t::disconnecting);
            m_connect = false;
        }

        return false;
    }

    void on_checking_timeout()
    {

    }

    bool on_message_packet(const i_message_packet& packet)
    {
        if (auto active_pair = m_active_pair)
        {
            return active_pair->send_packet(packet);
        }

        return false;
    }

    bool on_message_command(const i_message_command& command)
    {
        if (command.subclass() == message_net_class)
        {
            switch(command.command().command_id)
            {
                case ice_gathering_command_t::id:

                break;
                default:;
            }
        }
        return false;
    }

    bool on_message_sink(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::packet:
                return on_message_packet(static_cast<const i_message_packet&>(message));
            break;
            case message_category_t::command:
                return on_message_command(static_cast<const i_message_command&>(message));
            break;
        }

        return false;
    }

    inline std::string get_password() const
    {
        return m_ice_params.local_endpoint.auth.password;
    }


    inline void on_socket_local_candidate(ice_socket_t& socket
                                          , const ice_candidate_t& candidate)
    {

    }

    inline void on_remote_candidate(ice_socket_t& socket
                                    , const ice_candidate_t& candidate)
    {

    }

    inline void on_socket_gathering_state(ice_socket_t& socket
                                          , ice_gathering_state_t gathering_state)
    {

    }

    bool on_ice_request(ice_socket_t& socket
                        , ice_transaction_t &transaction)
    {
        if (transaction.request.username == m_ice_params.local_username())
        {
            return true;
        }

        return false;
    }

    bool on_ice_response(ice_socket_t& socket
                         , ice_transaction_t &transaction)
    {

        return false;
    }

    bool on_pair_state(candidate_pair_t& pair
                       , ice_state_t state)
    {
        return true;
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
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
        auto ice_params = m_ice_params;
        if (utils::property::deserialize(ice_params
                                          , params))
        {
            ice_params.local_endpoint = m_ice_params.local_endpoint;
            return true;

        }

        return false;
    }

    bool get_params(i_property &params) const override
    {
        return utils::property::serialize(m_ice_params
                                          , params);
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
        return transport_id_t::ice;
    }

    // i_ice_transport interface
public:
    ice_component_id_t component_id() const override
    {
        return m_ice_params.component_id;
    }

    ice_mode_t mode() const override
    {
        return m_ice_params.mode;
    }

    ice_gathering_state_t gathering_state() const override
    {
        return m_gathering_state;
    }

    ice_endpoint_t local_endpoint() const override
    {
        return m_ice_params.local_endpoint;
    }

    ice_endpoint_t remote_endpoint() const override
    {
        return m_ice_params.remote_endpoint;
    }

    bool add_remote_candidate(const ice_candidate_t &candidate) override
    {
        return false;
    }
};

ice_transport_factory::u_ptr_t ice_transport_factory::create(const ice_config_t& ice_config
                                                             , i_transport_factory& socket_factory
                                                             , i_timer_manager &timer_manager)
{
    return std::make_unique<ice_transport_factory>(ice_config
                                                   , socket_factory
                                                   , timer_manager);
}

ice_transport_factory::ice_transport_factory(const ice_config_t& ice_config
                                             , i_transport_factory& socket_factory
                                             , i_timer_manager &timer_manager)
    : m_ice_config(ice_config)
    , m_socket_factory(socket_factory)
    , m_timer_manager(timer_manager)
{

}

i_transport_channel::u_ptr_t ice_transport_factory::create_transport(const i_property &params)
{
    return ice_transport_impl::create(m_ice_config
                                      , m_socket_factory
                                      , m_timer_manager
                                      , params);
}


}
