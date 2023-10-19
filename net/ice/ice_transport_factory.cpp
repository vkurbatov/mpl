#include "ice_transport_factory.h"
#include "i_ice_transport.h"

#include "core/event_channel_state.h"

#include "utils/property_utils.h"
#include "utils/property_writer.h"
#include "utils/message_sink_impl.h"
#include "utils/message_router_impl.h"
#include "utils/message_event_impl.h"
#include "utils/pointer_utils.h"
#include "utils/common_utils.h"
#include "utils/enum_utils.h"

#include "net/stun/stun_packet_impl.h"
#include "net/stun/stun_error_codes.h"

#include "net/net_message_types.h"
#include "net/socket/socket_packet_impl.h"
#include "net/net_utils.h"

#include "ice_gathering_state_event.h"
#include "ice_transport_params.h"
#include "ice_controller.h"

#include "tools/base/sync_base.h"
#include "tools/io/net/net_utils.h"

#include <shared_mutex>
#include <list>
#include <unordered_map>
#include <iostream>

template<>
struct std::hash<mpl::net::socket_address_t>
{
    std::size_t operator()(const mpl::net::socket_address_t& s) const noexcept
    {
        return s.hash();
    }
};


namespace mpl::net
{

namespace detail
{

socket_address_t::array_t extract_address_list(const socket_address_t& local_address)
{
    socket_address_t::array_t address_list;

    if (!local_address.address.is_any())
    {
        address_list.push_back(local_address);
    }
    else
    {
        for (const auto& a : io::utils::get_net_info(net::ip_version_t::ip4))
        {
            if (!a.ip_address.is_loopback())
            {
                address_list.emplace_back(a.ip_address
                                         , local_address.port);
            }
        }
    }

    return address_list;
}

socket_address_t get_stun_address(const ice_server_params_t& stun_param)
{
    socket_address_t address(stun_param.get_dns_name());
    if (address.is_valid())
    {
        if (address.port == port_any)
        {
            address.port = port_stun;
        }
    }

    return address;
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
        // using pair_set_t = std::set<candidate_pair_t*>;

        struct hasher_t
        {
            std::size_t operator()(const socket_address_t& s) const noexcept
            {
                return std::hash<io::ip_endpoint_t>()(static_cast<const io::ip_endpoint_t&>(s));
            }
        };

        using pair_map_t = std::unordered_map<socket_address_t, candidate_pair_t*>;

        ice_transport_impl&             m_owner;
        i_socket_transport::u_ptr_t     m_socket;
        message_sink_impl               m_socket_sink;

        ice_candidate_t::array_t        m_local_candidates;
        socket_address_t                m_mapped_address;

        pair_map_t                      m_pairs;

        ice_controller                  m_ice_controller;

        i_timer::u_ptr_t                m_gathering_timer;
        ice_gathering_state_t           m_gathering_state;

        bool                            m_established;

        ice_socket_t(ice_transport_impl& owner
                     , i_socket_transport::u_ptr_t&& socket)
            : m_owner(owner)
            , m_socket(std::move(socket))
            , m_socket_sink([&](auto&& ...args) { return on_socket_message(args...); })
            , m_ice_controller(*this
                               , m_owner.m_timer_manager)
            , m_gathering_timer(m_owner.m_timer_manager.create_timer([&] { on_gathering_completed(); }))
            , m_gathering_state(ice_gathering_state_t::ready)
            , m_established(false)
        {
            m_socket->source(0)->add_sink(&m_socket_sink);
        }

        ~ice_socket_t()
        {
            control(channel_control_t::close());
            change_gathering_state(ice_gathering_state_t::closed);
            m_socket->source(0)->remove_sink(&m_socket_sink);
        }

        inline bool add_pair(candidate_pair_t* pair)
        {
            return m_pairs.emplace(pair->m_remote_candidate.connection_address
                                   , pair).second;
        }

        inline bool remove_pair(candidate_pair_t* pair)
        {
            if (auto it = m_pairs.find(pair->m_remote_candidate.connection_address)
                    ; it != m_pairs.end())
            {
                if (it->second == pair)
                {
                    m_pairs.erase(it);
                    return true;
                }
            }
            return false;
        }


        inline std::uint32_t priority() const
        {
            if (!m_local_candidates.empty())
            {
                return m_local_candidates.front().priority;
            }
            return 0;
        }

        inline socket_type_t socket_type() const
        {
            return m_socket->transport_id() == transport_id_t::tcp
                    ? socket_type_t::tcp
                    : socket_type_t::udp;
        }

        inline void set_established(bool established)
        {
            if (m_established != established)
            {
                m_established = established;
                m_owner.on_socket_established(*this
                                              , established);
            }
        }

        inline void change_gathering_state(ice_gathering_state_t new_state)
        {
            if (m_gathering_state != new_state)
            {
                m_gathering_state = new_state;
                m_owner.on_socket_gathering_state(*this
                                                  , new_state);
            }
        }  

        inline bool add_candidate(const socket_address_t& address
                                  , ice_candidate_type_t candidate_type = ice_candidate_type_t::host)
        {
            if (ice_candidate_t::find(m_local_candidates
                                     , address
                                     , m_socket->transport_id()) == nullptr)
            {
                auto candidate = m_owner.add_local_candidate(address
                                                             , m_socket->transport_id()
                                                             , candidate_type
                                                             , m_socket->local_endpoint().socket_address);
                if (candidate.is_valid())
                {
                    if (m_owner.on_socket_candidate(*this
                                                    , candidate))
                    {
                        m_local_candidates.emplace_back(candidate);
                    };
                }

                return true;
            }

            return false;
        }

        inline bool on_mapped_address(const socket_address_t& mapped_address)
        {
            if (m_mapped_address != mapped_address)
            {
                m_mapped_address = mapped_address;
                add_candidate(mapped_address
                              , ice_candidate_type_t::srflx);

                return true;
            }

            return false;
        }

        inline const ice_config_t& ice_config() const
        {
            return m_owner.m_ice_config;
        }

        bool send_transaction(ice_transaction_t&& transaction
                              , bool flush = true)
        {
            if (is_established())
            {
                if (flush)
                {
                    m_ice_controller.reset(transaction.tag);
                }

                return m_ice_controller.send_request(std::move(transaction));
            }

            return false;
        }

        inline void start_gathering(const ice_server_params_t::array_t& servers)
        {
            stop_gathering();

            if (is_established())
            {
                change_gathering_state(ice_gathering_state_t::gathering);
                bool result = false;

                m_gathering_timer->start(m_owner.m_ice_config.ice_check_interval());

                for (const auto& s : servers)
                {
                    auto transaction = m_owner.create_stun_request(s);
                    if (transaction.address.is_valid())
                    {
                        result |= send_transaction(std::move(transaction)
                                                   , false);
                    }
                }

                if (result == false)
                {
                    stop_gathering();
                }
            }
        }

        void on_gathering_completed()
        {
            m_ice_controller.reset(ice_gathering_id);
            change_gathering_state(ice_gathering_state_t::completed);
        }

        inline void stop_gathering()
        {
            m_gathering_timer->stop();
            on_gathering_completed();
        }

        inline bool control(const channel_control_t& channel_control)
        {
            return m_socket->control(channel_control);
        }

        bool send_packet(const i_message_packet& packet
                         , const socket_address_t& address)
        {
            if (is_established())
            {
                if (auto sink = m_socket->sink(0))
                {
                    socket_packet_impl socket_packet(smart_buffer(&packet)
                                                     , socket_endpoint_t(socket_type()
                                                                         , address));

                    return sink->send_message(socket_packet);
                }
            }

            return false;
        }

        inline candidate_pair_t* get_pair(const socket_address_t& address)
        {
            if (auto it = m_pairs.find(address)
                    ; it != m_pairs.end())
            {
                return it->second;
            }

            return nullptr;
        }

        inline candidate_pair_t* get_or_append_pair(const socket_address_t& address
                                                    , std::uint32_t priority = 0)
        {
            if (auto pair = get_pair(address))
            {
                return pair;
            }

            ice_candidate_t new_candidate(std::to_string(utils::random<std::uint32_t>())
                                          , m_owner.component_id()
                                          , m_socket->transport_id()
                                          , priority
                                          , address
                                          , ice_candidate_type_t::prflx);

            if (m_owner.internal_add_remote_candidate(new_candidate))
            {
                return get_pair(address);
            }

            return nullptr;
        }

        inline bool on_socket_state(channel_state_t new_state
                                    , const std::string_view& reason)
        {
            switch(new_state)
            {
                case channel_state_t::open:
                {
                    m_local_candidates.clear();
                    m_mapped_address = socket_address_t::undefined();
                    for (const auto& a : detail::extract_address_list(m_socket->local_endpoint().socket_address))
                    {
                        add_candidate(a
                                      , ice_candidate_type_t::host);
                    }
                }
                break;
                default:;
            }


            set_established(new_state == channel_state_t::connected);

            m_owner.on_socket_channel_state(*this
                                            , new_state
                                            , reason);


            return true;
        }

        inline bool on_socket_event(const i_message_event& event)
        {
            if (event.subclass() == message_class_core
                    && event.event().event_id == event_channel_state_t::id)
            {
                return on_socket_state(static_cast<const event_channel_state_t&>(event.event()).state
                                       , static_cast<const event_channel_state_t&>(event.event()).reason);
            }

            return false;
        }

        inline bool on_socket_packet(const i_message_packet& packet)
        {
            if (packet.subclass() == message_class_net)
            {
                auto& socket_packet = static_cast<const i_socket_packet&>(packet);
                switch(utils::parse_protocol(packet.data()
                                             , packet.size()))
                {
                    case protocol_type_t::stun:
                    {
                        stun_packet_impl stun_packet(smart_buffer(&packet)
                                                     , socket_packet.endpoint().socket_address);

                        return m_ice_controller.push_packet(stun_packet);
                    }
                    break;
                    default:
                    {
                        if (m_owner.m_ice_params.mode == ice_mode_t::undefined)
                        {
                            if (auto pair = get_or_append_pair(socket_packet.endpoint().socket_address))
                            {
                                m_owner.set_active_pair(pair);
                                return pair->on_socket_packet(socket_packet);
                            }
                        }
                        else if (auto pair = get_pair(socket_packet.endpoint().socket_address))
                        {
                            return pair->on_socket_packet(socket_packet);
                        }
                    };
                }
            }

            return false;
        }

        inline bool on_socket_message(const i_message& socket_message)
        {
            switch(socket_message.category())
            {
                case message_category_t::event:
                {
                    return on_socket_event(static_cast<const i_message_event&>(socket_message));
                }
                break;
                case message_category_t::packet:
                {
                    return on_socket_packet(static_cast<const i_message_packet&>(socket_message));
                }
                break;
                default:;
            }

            return false;
        }


        inline bool is_established() const
        {
            return m_established;
        }

        inline channel_state_t state() const
        {
            return m_socket->state();
        }

        // i_listener interface
    public:
        bool on_send_packet(const i_stun_packet &stun_packet) override
        {
            return send_packet(stun_packet
                               , stun_packet.address());
        }

        std::string on_autorized(const ice_transaction_t &transaction) override
        {
            return m_owner.get_password();
        }

        bool on_request(ice_transaction_t &transaction) override
        {
            if (auto pair = get_or_append_pair(transaction.address
                                               , transaction.request.priority))
            {
                return m_owner.on_pair_ice_transaction(*pair
                                                       , transaction
                                                       , true);
            }

            return false;
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

                        return true;
                    }
                }
                break;
                default:
                {
                    if (auto pair = get_pair(transaction.address))
                    {
                        return m_owner.on_pair_ice_transaction(*pair
                                                               , transaction
                                                               , false);
                    }
                }
            }

            return false;
        }
    };

    struct candidate_pair_t
    {
        using list_t = std::list<candidate_pair_t>;
        using iter_t = list_t::iterator;

        ice_transport_impl&         m_owner;
        ice_socket_t&               m_socket;
        ice_candidate_t             m_remote_candidate;
        bool                        m_recheck;

        ice_state_t                 m_state;

        candidate_pair_t(ice_socket_t& socket
                         , const ice_candidate_t& candidate)
            : m_owner(socket.m_owner)
            , m_socket(socket)
            , m_remote_candidate(candidate)
            , m_recheck(false)
            , m_state(ice_state_t::frozen)
        {
            m_socket.add_pair(this);
        }

        ~candidate_pair_t()
        {
            m_socket.remove_pair(this);
            if (is_active())
            {
                m_owner.set_active_pair(nullptr);
            }

            if (is_checking())
            {
                m_owner.m_checking_pair = nullptr;
            }

            set_state(ice_state_t::shutdown);
        }

        inline void set_state(ice_state_t new_state)
        {
            if (m_state != new_state)
            {
                m_state = new_state;
                m_owner.on_pair_state(*this
                                      , new_state);
            }
        }

        inline bool send_packet(const i_message_packet& packet)
        {
            return m_socket.send_packet(packet
                                        , m_remote_candidate.connection_address);
        }

        inline bool send_check()
        {
            return m_socket.send_transaction(m_owner.create_ice_request(*this));
        }

        bool checking()
        {
            bool one_flag = true;
            m_recheck = false;

            while(true)
            {
                switch(m_state)
                {
                    case ice_state_t::frozen:
                        set_state(ice_state_t::waiting);
                    break;
                    case ice_state_t::waiting:
                        set_state(ice_state_t::in_progress);
                    break;
                    case ice_state_t::in_progress:
                    case ice_state_t::succeeded:
                        if (send_check())
                        {
                            return true;
                        }
                        set_state(ice_state_t::failed);
                    break;
                    case ice_state_t::failed:
                        if (one_flag)
                        {
                            one_flag = false;
                            set_state(ice_state_t::frozen);
                        }
                        else
                        {
                            return false;
                        }
                    break;
                    default:
                        return false;
                }
            }

            return false;
        }

        inline bool operator == (const candidate_pair_t& other) const
        {
            return this == &other;
        }

        inline bool operator < (const candidate_pair_t& other) const
        {
            return priority() < other.priority();
        }

        inline bool operator > (const candidate_pair_t& other) const
        {
            return priority() > other.priority();
        }

        std::uint64_t priority() const
        {
            std::uint64_t g_priority = m_socket.priority();
            std::uint64_t d_priority = m_remote_candidate.priority;

            if (!m_owner.m_controlling)
            {
                std::swap(g_priority
                          , d_priority);
            }

            // rfc8445: pair priority = 2^32*MIN(G,D) + 2*MAX(G,D) + (G>D?1:0)

            return 0x100000000ul * std::min(g_priority, d_priority)
                    + 2 * std::max(g_priority, d_priority)
                    + (g_priority > d_priority ? 1 : 0);
        }

        inline bool is_active() const
        {
            return m_owner.m_active_pair == this;
        }

        inline bool is_checking() const
        {
            return m_owner.m_checking_pair == this;
        }

        bool on_socket_packet(const i_socket_packet& packet)
        {
            return m_owner.on_pair_packet(*this
                                          , packet);
        }
    };

    mutable mutex_t                     m_safe_mutex;

    std::uint16_t                       m_foundation_id;

    ice_config_t                        m_ice_config;
    i_transport_factory&                m_socket_factory;
    i_timer_manager&                    m_timer_manager;
    mutable ice_transport_params_t      m_ice_params;

    ice_socket_t::list_t                m_sockets;

    message_sink_impl                   m_message_sink;
    message_router_impl                 m_router;
    i_timer::u_ptr_t                    m_checking_timer;


    candidate_pair_t::list_t            m_pairs;
    candidate_pair_t*                   m_active_pair;
    candidate_pair_t*                   m_checking_pair;

    std::uint64_t                       m_tie_breaker;
    bool                                m_controlling;

    bool                                m_open;
    bool                                m_connect;
    channel_state_t                     m_state;
    ice_gathering_state_t               m_gathering_state;

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

        return nullptr;
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
        , m_message_sink([&](auto&& args) { return on_send_message(args); })
        , m_checking_timer(m_timer_manager.create_timer([&]{ on_checking_timeout(); }))
        , m_active_pair(nullptr)
        , m_checking_pair(nullptr)
        , m_tie_breaker(utils::random<std::uint64_t>())
        , m_controlling(false)
        , m_open(false)
        , m_connect(false)
        , m_state(channel_state_t::ready)
        , m_gathering_state(ice_gathering_state_t::ready)
    {
        // initialize();
    }

    ~ice_transport_impl()
    {
        close();
    }

    bool initialize()
    {
        m_foundation_id = 0;
        set_active_pair(nullptr);
        m_ice_params.local_endpoint.candidates.clear();
        m_pairs.clear();
        m_sockets.clear();

        for (auto it = m_ice_params.sockets.begin()
             ; it != m_ice_params.sockets.end()
             ; )
        {
            if (auto socket = create_socket(*it))
            {
                m_sockets.emplace_back(*this
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
        // 1. remove undefined pairs

        for (auto it = m_pairs.begin()
             ; it != m_pairs.end()
             ; )
        {
            bool remove = true;
            for (const auto& c : remote_candidates)
            {
                if (it->m_remote_candidate.is_equal_endpoint(c))
                {
                    it->m_remote_candidate = c;
                    remove = false;
                    break;
                }
            }

            if (remove)
            {
                if (m_active_pair == &(*it))
                {
                    set_active_pair(nullptr);
                }
                it = m_pairs.erase(it);
            }
            else
            {
                it = std::next(it);
            }
        }

        // 2. add new pairs
        for (const auto& c : remote_candidates)
        {
            if (auto it = std::find_if(m_pairs.begin()
                                       , m_pairs.end()
                                       , [&c](auto&& args) { return args.m_remote_candidate == c; })
                                       ; it != m_pairs.end()
                    )
            {
                continue;
            }

            for (auto& s : m_sockets)
            {
                m_pairs.emplace_back(s
                                     , c);
            }
        }

        m_pairs.sort(std::greater<candidate_pair_t>());

    }

    inline void update_pairs()
    {
        update_pairs(m_ice_params.remote_endpoint.candidates);
    }

    candidate_pair_t* next_pair(candidate_pair_t* pair)
    {
        if (m_active_pair != nullptr)
        {
            return m_active_pair;
        }
        if (!m_pairs.empty())
        {
            if (pair == nullptr)
            {
                return &m_pairs.front();
            }

            candidate_pair_t::iter_t it = std::find_if(m_pairs.begin()
                                                       , m_pairs.end()
                                                       , [pair] (const auto& p) { return pair == &p; } );
            if (it != m_pairs.end())
            {
                if (!it->m_recheck)
                {
                    it = std::next(it);
                }
            }

            if (it != m_pairs.end())
            {
                return &(*it);
            }
        }

        return nullptr;
    }

    inline void set_checking_pair(candidate_pair_t* checking_pair)
    {
        if (m_checking_pair != checking_pair)
        {
            if (m_checking_pair != nullptr)
            {
                if (m_checking_pair->m_state != ice_state_t::succeeded)
                {
                    m_checking_pair->set_state(ice_state_t::frozen);
                }
            }

            m_checking_pair = checking_pair;
        }
    }

    void set_active_pair(candidate_pair_t* active_pair)
    {
        if (m_active_pair != active_pair)
        {
            if (m_active_pair != nullptr)
            {
                m_active_pair->set_state(ice_state_t::frozen);
                change_channel_state(channel_state_t::disconnected
                                     , m_active_pair->m_remote_candidate.to_string());
            }


            m_active_pair = active_pair;

            if (m_active_pair != nullptr)
            {
                set_checking_pair(nullptr);
                change_channel_state(channel_state_t::connected
                                     , m_active_pair->m_remote_candidate.to_string());
            }
        }
    }

    inline i_socket_transport::u_ptr_t create_socket(const socket_endpoint_t& socket_endpoint)
    {
        if (auto socket_params = utils::property::create_property(property_type_t::object))
        {
            property_writer writer(*socket_params);
            writer.set("local_endpoint", socket_endpoint);
            writer.set("options.reuse_address", true);

            if (auto socket = utils::static_pointer_cast<i_socket_transport>(m_socket_factory.create_transport(*socket_params)))
            {
                if (socket->transport_id() == socket_endpoint.transport_id)
                {
                    return socket;
                }
            }
        }

        return nullptr;
    }

    inline std::string get_foundation(const socket_address_t& address
                                       , transport_id_t transport_id
                                       , ice_candidate_type_t candidate_type
                                       , const socket_address_t& relayed_address)
    {
        for (const auto& c : m_ice_params.local_endpoint.candidates)
        {
            if (c.connection_address.address == address.address
                    && c.transport == transport_id
                    && c.type == candidate_type
                    && c.relayed_address.address == relayed_address.address)
            {
                return c.foundation;
            }
        }

        return std::to_string(m_foundation_id++);
    }

    inline ice_candidate_t add_local_candidate(const socket_address_t& connection_address
                                                        , transport_id_t transport_id = transport_id_t::udp
                                                        , ice_candidate_type_t candidate_type = ice_candidate_type_t::host
                                                        , const socket_address_t& relayed_address = {})
    {
        lock_t lock(m_safe_mutex);
        if (auto candidate = ice_candidate_t::find(m_ice_params.local_endpoint.candidates
                                                   , connection_address
                                                   , transport_id))
        {
            return *candidate;
        }

        auto new_candidate = ice_candidate_t::build_candidate(get_foundation(connection_address.address
                                                                             , transport_id
                                                                             , candidate_type
                                                                             , relayed_address)
                                                              , m_ice_params.local_endpoint.candidates.size()
                                                              , m_ice_params.component_id
                                                              , connection_address
                                                              , transport_id
                                                              , candidate_type
                                                              , relayed_address);

        if (new_candidate.is_valid())
        {
            m_ice_params.local_endpoint.candidates.push_back(new_candidate);
        }
        return new_candidate;
    }


    inline bool has_role_conflict(bool controlling) const
    {
        switch(m_ice_params.mode)
        {
            case ice_mode_t::aggressive:
            case ice_mode_t::regular:
                return m_controlling == controlling;
            break;
            default:
                return !controlling;
        }

        return false;
    }

    void change_gathering_state(ice_gathering_state_t new_state
                                , const std::string_view& reason = {})
    {
        if (m_gathering_state != new_state)
        {
            m_gathering_state = new_state;
            m_router.send_message(message_event_impl<ice_gathering_state_event_t
                                  , message_class_net>(ice_gathering_state_event_t(new_state
                                                                                 , reason))
                                  );
        }
    }

    void change_channel_state(channel_state_t new_state
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


    bool do_checking()
    {
        if (m_connect
                && m_ice_params.is_full())
        {
            bool result = false;

            set_checking_pair(next_pair(m_checking_pair));

            if (m_checking_pair != nullptr)
            {
                if (m_active_pair == nullptr)
                {
                    change_channel_state(channel_state_t::connecting);
                }

                result = m_checking_pair->checking();
            }

            start_checking(m_ice_config.ice_check_interval());
            return result;
        }

        return false;
    }

    inline bool stop_checking()
    {
        m_checking_timer->stop();
        set_active_pair(nullptr);
        return true;
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
            m_ice_params.local_endpoint.candidates.clear();
            change_channel_state(channel_state_t::opening);

            if (initialize())
            {
                m_open = true;

                if (control_sockets(channel_control_t::open()) > 0)
                {
                    update_pairs();
                    m_ice_params.sockets = get_local_addresses();
                    change_channel_state(channel_state_t::open);
                    return true;
                }

                change_channel_state(channel_state_t::failed);
                m_open = false;
            }
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
            start_checking();
            return true;
        }

        return false;
    }

    bool shutdown()
    {
        if (m_open
                && m_connect)
        {
            change_channel_state(channel_state_t::disconnecting);
            stop_checking();
            m_connect = false;
        }

        return false;
    }

    bool on_message_packet(const i_message_packet& packet)
    {
        if (auto active_pair = m_active_pair)
        {
            return active_pair->send_packet(packet);
        }

        return false;
    }

    bool on_send_message(const i_message& message)
    {
        shared_lock_t lock(m_safe_mutex);
        switch(message.category())
        {
            case message_category_t::packet:
                return on_message_packet(static_cast<const i_message_packet&>(message));
            break;
        }

        return false;
    }

    inline std::string get_password() const
    {
        return m_ice_params.local_endpoint.auth.password;
    }

    void update_transaction_params(ice_transaction_t& transaction)
    {
        transaction.retries = m_ice_config.retry_count;
        transaction.timeout = m_ice_config.ice_timeout;
    }

    ice_transaction_t create_stun_request(const ice_server_params_t& stun_server)
    {
        ice_transaction_t request;

        request.tag = ice_gathering_id;
        request.address = detail::get_stun_address(stun_server);

        if (request.address.is_valid())
        {
             update_transaction_params(request);
        }

        return request;
    }

    ice_transaction_t create_ice_request(candidate_pair_t& pair)
    {
        ice_transaction_t request;


        request.tag = ice_binding_id;
        request.address = pair.m_remote_candidate.connection_address;
        request.request.username = m_ice_params.remote_username();
        request.request.password = m_ice_params.remote_endpoint.auth.password;


        request.request.tie_breaker = m_tie_breaker;
        request.request.priority = pair.m_socket.priority();
        request.request.controlling = m_controlling;
        auto use_candidate = request.request.controlling && (pair.m_state == ice_state_t::succeeded || m_ice_params.mode == ice_mode_t::aggressive);
        request.request.use_candidate = use_candidate;

        update_transaction_params(request);

        return request;
    }

    bool internal_add_remote_candidate(const ice_candidate_t& candidate)
    {     
        if (candidate.is_valid()
                && candidate.component_id == m_ice_params.component_id)
        {
            lock_t lock(m_safe_mutex);
            if (ice_candidate_t::find(m_ice_params.remote_endpoint.candidates
                                       , candidate.connection_address
                                       , candidate.transport) == nullptr)
            {
                m_ice_params.remote_endpoint.candidates.emplace_back(candidate);
                update_pairs(m_ice_params.remote_endpoint.candidates);
                return true;
            }
        }

        return false;
    }

    inline socket_endpoint_t::array_t get_local_addresses() const
    {
        socket_endpoint_t::array_t socket_addresses;

        for (const auto& s : m_sockets)
        {
            socket_addresses.push_back(s.m_socket->local_endpoint());
        }

        return socket_addresses;
    }

    inline ice_candidate_t::array_t get_local_candidates() const
    {
        ice_candidate_t::array_t candidates;
        for (const auto& s : m_sockets)
        {
            candidates.insert(candidates.begin()
                              , s.m_local_candidates.begin()
                              , s.m_local_candidates.end());
        }

        return candidates;
    }

    inline void on_socket_gathering_state(ice_socket_t& socket
                                          , ice_gathering_state_t gathering_state)
    {
        if (gathering_state == ice_gathering_state_t::completed)
        {
            for (const auto& s : m_sockets)
            {
                if (s.m_gathering_state != gathering_state)
                {
                    return;
                }
            }
            change_gathering_state(gathering_state);
        }
        else if (&socket == &m_sockets.front())
        {
            change_gathering_state(gathering_state);
        }
    }

    inline void on_socket_channel_state(ice_socket_t& socket
                                          , channel_state_t channel_state
                                        , const std::string_view& reason)
    {
        if (channel_state == channel_state_t::open
                && socket.socket_type() == socket_type_t::udp)
        {
            socket.control(channel_control_t::connect());
        }
    }


    inline bool on_socket_candidate(ice_socket_t& socket
                                    , const ice_candidate_t& candidate)
    {
        return true;
    }

    inline void on_socket_established(ice_socket_t& socket
                                      , bool established)
    {
        if (established)
        {
            socket.start_gathering(m_ice_config.ice_servers);
        }
        else
        {
            return;
        }
    }

    void on_pair_ice_request(candidate_pair_t& pair
                             , ice_transaction_t& transaction)
    {
        bool change_role = false;
        if (has_role_conflict(transaction.request.controlling))
        {
            bool need_switch_role = !m_ice_params.is_full()
                    || m_controlling == (m_tie_breaker >= transaction.request.tie_breaker);
            if (need_switch_role)
            {
                transaction.response.error_code = error_role_conflict;
                transaction.response.result = ice_transaction_t::ice_result_t::failed;
            }

            change_role = true;
            m_controlling ^= true;
        }

        transaction.response.result = ice_transaction_t::ice_result_t::success;

        if (transaction.request.use_candidate)
        {
            set_active_pair(&pair);
        }
        else
        {
            if (change_role)
            {
                pair.m_recheck = true;
                set_checking_pair(&pair);
                start_checking();
            }
        }
    }

    void on_pair_ice_response(candidate_pair_t& pair
                             , ice_transaction_t& transaction)
    {
        bool is_active = pair.is_active();
        bool is_checking = pair.is_checking();

        if (is_active
                || is_checking)
        {
            if (transaction.response.is_success())
            {
                pair.set_state(ice_state_t::succeeded);

                if (transaction.request.use_candidate)
                {
                    set_active_pair(&pair);
                }
                else
                {
                    if (transaction.request.controlling)
                    {
                        pair.m_recheck = true;
                    }
                }

                if (is_active)
                {
                    return;
                }
            }
            else
            {
                if (is_active)
                {
                    set_active_pair(nullptr);
                }

                if (transaction.response.result == ice_transaction_t::ice_result_t::failed)
                {
                    switch(transaction.response.error_code)
                    {
                        case error_role_conflict:
                        {
                            m_controlling ^= true;
                            pair.m_recheck = is_checking;
                        }
                        break;
                        default:
                            pair.set_state(ice_state_t::failed);
                    }
                }
            }

            start_checking();
        }
        else
        {
            pair.set_state(ice_state_t::frozen);
        }
    }

    inline bool on_pair_ice_transaction(candidate_pair_t& pair
                                 , ice_transaction_t& transaction
                                 , bool request)
    {
        if (m_connect)
        {
            lock_t lock(m_safe_mutex);

            if (request)
            {
                on_pair_ice_request(pair
                                    , transaction);
            }
            else
            {
                on_pair_ice_response(pair
                                     , transaction);

            }

            return true;
        }

        return false;
    }

    inline void on_checking_timeout()
    {
        lock_t lock(m_safe_mutex);
        do_checking();
    }

    inline void start_checking(timestamp_t timeout = 0)
    {
        switch(m_ice_params.mode)
        {
            case ice_mode_t::aggressive:
            case ice_mode_t::regular:
            {
                m_checking_timer->start(timeout);
            }
            break;
            case ice_mode_t::lite:

            break;
            default:
            {
                set_active_pair(next_pair(nullptr));
            }
        }
    }

    inline void on_pair_state(candidate_pair_t& pair
                              , ice_state_t state)
    {

    }

    inline bool on_pair_packet(candidate_pair_t& pair
                                , const i_socket_packet& packet)
    {
        if (m_active_pair == &pair)
        {
            return m_router.send_message(packet);
        }

        return false;
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
        lock_t lock(m_safe_mutex);
        auto ice_params = m_ice_params;
        if (utils::property::deserialize(ice_params
                                          , params))
        {
            ice_params.sockets = m_ice_params.sockets;
            ice_params.local_endpoint.candidates = m_ice_params.local_endpoint.candidates;

            if (ice_params != m_ice_params)
            {
                m_ice_params = ice_params;

                update_pairs(m_ice_params.remote_endpoint.candidates);

                start_checking();
            }
            return true;
        }

        return false;
    }

    bool get_params(i_property &params) const override
    {
        lock_t lock(m_safe_mutex);
        auto ice_params = m_ice_params;
        ice_params.sockets.clear();
        ice_params.sockets = get_local_addresses();

        return utils::property::serialize(ice_params
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
        shared_lock_t lock(m_safe_mutex);
        return m_ice_params.local_endpoint;
    }

    ice_endpoint_t remote_endpoint() const override
    {
        shared_lock_t lock(m_safe_mutex);
        return m_ice_params.remote_endpoint;
    }

    bool set_local_endpoint(const ice_endpoint_t &local_endpoint) override
    {
        lock_t lock(m_safe_mutex);
        m_ice_params.local_endpoint.auth = local_endpoint.auth;
        return true;
    }

    bool set_remote_endpoint(const ice_endpoint_t &remote_endpoint) override
    {
        lock_t lock(m_safe_mutex);
        m_ice_params.remote_endpoint = remote_endpoint;
        update_pairs();
        return true;
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
