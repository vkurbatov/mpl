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

        inline const ice_candidate_t& candidate() const
        {
            return m_local_candidates.front();
        }

        inline std::uint32_t priority() const
        {
            return candidate().priority;
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
                auto candidate = m_owner.create_local_candidate(address
                                                                , m_socket->transport_id()
                                                                , candidate_type);

                if (candidate_type != ice_candidate_type_t::host)
                {
                    candidate.relayed_address = m_socket->local_endpoint().socket_address;
                }

                if (candidate.is_valid())
                {
                    m_local_candidates.emplace_back(candidate);
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

                m_gathering_timer->start(m_owner.m_ice_config.allocate_timeout);

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
            if (event.subclass() == message_core_class
                    && event.event().event_id == event_channel_state_t::id)
            {
                return on_socket_state(static_cast<const event_channel_state_t&>(event.event()).state
                                       , static_cast<const event_channel_state_t&>(event.event()).reason);
            }

            return false;
        }

        inline bool on_socket_packet(const i_message_packet& packet)
        {
            if (packet.subclass() == message_net_class)
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
                        if (auto pair = get_pair(socket_packet.endpoint().socket_address))
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
            if (auto pair = get_pair(transaction.address))
            {
                return pair->on_ice_request(transaction);
            }
            else
            {
                ice_candidate_t new_candidate(std::to_string(utils::random<std::uint32_t>())
                                              , m_owner.component_id()
                                              , m_socket->transport_id()
                                              , transaction.request.priority
                                              , transaction.address
                                              , ice_candidate_type_t::prflx);

                if (m_owner.add_remote_candidate(new_candidate))
                {
                    if (auto pair = get_pair(transaction.address))
                    {
                        return pair->on_ice_request(transaction);
                    }
                }
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
                    }
                }
                break;
                default:
                {
                    if (auto pair = get_pair(transaction.address))
                    {
                        return pair->on_ice_response(transaction);
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

        ice_state_t                 m_state;

        candidate_pair_t(ice_socket_t& socket
                         , const ice_candidate_t& candidate)
            : m_owner(socket.m_owner)
            , m_socket(socket)
            , m_remote_candidate(candidate)
            , m_state(ice_state_t::waiting)
        {
            m_socket.add_pair(this);
        }

        ~candidate_pair_t()
        {
            m_socket.remove_pair(this);
            set_state(ice_state_t::shutdown);
        }

        inline void set_state(ice_state_t new_state
                              , bool use_candidate = false)
        {
            if (m_state != new_state)
            {
                m_state = new_state;
                m_owner.on_pair_state(*this
                                      , new_state
                                      , use_candidate);
            }
        }


        inline bool send_packet(const i_message_packet& packet)
        {
            return m_socket.send_packet(packet
                                        , m_remote_candidate.connection_address);
        }

        bool checking()
        {
            set_state(ice_state_t::in_progress);
            if (m_socket.send_transaction(m_owner.create_ice_request(*this)))
            {
                return true;
            }

            set_state(ice_state_t::failed);
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

        bool on_ice_request(ice_transaction_t &transaction)
        {
            if (m_socket.is_established())
            {
                if (transaction.request.username == m_owner.m_ice_params.local_username())
                {
                    if (!m_owner.process_role_conflict(transaction))
                    {
                        transaction.response.result = ice_transaction_t::ice_result_t::success;

                        set_state(ice_state_t::succeeded);
                    }

                    return true;
                }
            }

            return false;
        }

        bool on_ice_response(ice_transaction_t &transaction)
        {
            if (!transaction.response.is_success())
            {
                if (transaction.response.result == ice_transaction_t::ice_result_t::failed)
                {
                    switch(transaction.response.error_code)
                    {
                        case error_role_conflict:
                        {
                            m_owner.m_controlling ^= true;
                            transaction.request.controlling = m_owner.m_controlling;
                            transaction.request.use_candidate &= transaction.request.controlling;
                            m_owner.update_transaction_params(transaction);
                            return m_socket.send_transaction(std::move(transaction));
                        }
                        break;
                        default:;
                    }
                }
                set_state(ice_state_t::failed);
            }
            else
            {
                set_state(ice_state_t::succeeded);

                if (transaction.request.controlling
                        && !transaction.request.use_candidate)
                {
                    transaction.request.use_candidate = true;
                    m_owner.update_transaction_params(transaction);
                    m_socket.send_transaction(std::move(transaction));
                }

            }
            return true;
        }

        bool on_socket_packet(const i_socket_packet& packet)
        {
            return m_owner.on_pair_packet(*this
                                          , packet);
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


    candidate_pair_t::list_t    m_pairs;
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
        , m_message_sink([&](auto&& args) { return on_message_sink(args); })
        , m_checking_timer(m_timer_manager.create_timer([&]{ on_checking_timeout(); }))
        , m_active_pair(nullptr)
        , m_tie_breaker(utils::random<std::uint64_t>())
        , m_controlling(false)
        , m_open(false)
        , m_connect(false)
        , m_state(channel_state_t::ready)
        , m_gathering_state(ice_gathering_state_t::ready)
    {
        initialize();
        update_pairs(m_ice_params.remote_endpoint.candidates);
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

            for (const auto& c : remote_candidates)
            {
                if (it->m_remote_candidate.is_equal_endpoint(c))
                {
                    it->m_remote_candidate = c;
                    it = std::next(it);
                    break;
                }
            }
            if (m_active_pair == &(*it))
            {
                set_active_pair(nullptr);
            }
            it = m_pairs.erase(it);

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

        m_pairs.sort();

    }

    candidate_pair_t* next_pair(candidate_pair_t* pair)
    {
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
                it = std::next(it);
            }

            if (it == m_pairs.end())
            {
                it = m_pairs.begin();
            }

            return &(*it);
        }

        return nullptr;
    }

    void set_active_pair(candidate_pair_t* active_pair)
    {
        if (m_active_pair != active_pair)
        {
            if (m_active_pair != nullptr)
            {
                m_active_pair->set_state(ice_state_t::waiting);
                if (m_state == channel_state_t::connected)
                {
                    change_channel_state(channel_state_t::disconnected);
                }
            }

            m_active_pair = active_pair;
            if (m_active_pair != nullptr)
            {
                change_channel_state(channel_state_t::connected);
                start_timer(m_ice_config.ice_check_interval);
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

    inline ice_candidate_t create_local_candidate(const socket_address_t& connection_endpoint
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


    inline bool has_role_conflict(bool controlling) const
    {
        switch(m_ice_params.mode)
        {
            case ice_mode_t::agressive:
            case ice_mode_t::regular:
                return m_controlling == controlling;
            break;
            case ice_mode_t::lite:
                return !controlling;
            break;
            default:;
        }

        return false;
    }

    inline bool process_role_conflict(ice_transaction_t& transaction)
    {
        if (has_role_conflict(transaction.request.controlling))
        {
            bool need_switch_role = m_ice_params.mode == ice_mode_t::lite
                    || m_controlling == (m_tie_breaker >= transaction.request.tie_breaker);
            if (need_switch_role)
            {
                transaction.response.error_code = error_role_conflict;
                transaction.response.result = ice_transaction_t::ice_result_t::failed;
                return true;
            }

            m_controlling ^= true;
        }

        return false;
    }

    void change_gathering_state(ice_gathering_state_t new_state
                                , const std::string_view& reason = {})
    {
        if (m_gathering_state == new_state)
        {
            m_gathering_state = new_state;
            m_router.send_message(message_event_impl<ice_gathering_state_event_t
                                  , message_net_class>(ice_gathering_state_event_t(new_state
                                                                                 , reason))
                                  );
        }
    }

    void change_channel_state(channel_state_t new_state
                              , const std::string_view& reason = {})
    {
        if (m_state == new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl(event_channel_state_t(new_state
                                                                           , reason))
                                  );
        }

    }


    bool start_checking()
    {
        if (m_connect)
        {
            if (m_active_pair != nullptr)
            {
                start_timer(m_ice_config.ice_check_interval);
                m_active_pair->checking();
            }
            else
            {
                if (auto pair = next_pair(nullptr))
                {
                    return pair->checking();
                }
            }
        }

        return false;
    }

    inline bool stop_checking()
    {
        set_active_pair(nullptr);
        m_checking_timer->stop();
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
            start_checking();
            change_channel_state(channel_state_t::failed);
        }

        return false;
    }

    bool shutdown()
    {
        if (m_open
                && m_connect)
        {
            stop_checking();
            change_channel_state(channel_state_t::disconnecting);
            m_connect = false;
        }

        return false;
    }

    void on_checking_timeout()
    {
        if (m_connect)
        {
            if (m_active_pair == nullptr
                    || m_active_pair->m_state != ice_state_t::succeeded)
            {
                set_active_pair(next_pair(m_active_pair));
            }

            if (auto pair = m_active_pair)
            {
                pair->checking();
                return;
            }

            start_timer(m_ice_config.ice_check_interval);
        }
    }

    bool on_message_packet(const i_message_packet& packet)
    {
        if (auto active_pair = m_active_pair)
        {
            return active_pair->send_packet(packet);
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
        if (transaction.tag == ice_gathering_id)
        {
            transaction.timeout = m_ice_config.allocate_timeout;
        }
        else
        {
            transaction.timeout = m_ice_config.ice_timeout;
        }
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

    ice_transaction_t create_ice_request(candidate_pair_t& pair
                                        , bool use_candidate = false)
    {
        ice_transaction_t request;


        request.tag = ice_binding_id;
        request.address = pair.m_remote_candidate.connection_address;
        request.request.username = m_ice_params.remote_username();
        request.request.password = m_ice_params.remote_endpoint.auth.password;


        request.request.tie_breaker = m_tie_breaker;
        request.request.priority = pair.m_socket.priority();
        request.request.controlling = m_controlling;
        use_candidate = request.request.controlling && (use_candidate || m_ice_params.mode == ice_mode_t::agressive);
        request.request.use_candidate = use_candidate;

        update_transaction_params(request);

        return request;
    }

    bool internal_add_remote_candidate(const ice_candidate_t& candidate)
    {
        if (ice_candidate_t::find(m_ice_params.remote_endpoint.candidates
                                   , candidate.connection_address
                                   , candidate.transport) == nullptr)
        {
            m_ice_params.remote_endpoint.candidates.emplace_back(candidate);
            update_pairs(m_ice_params.remote_endpoint.candidates);
            return true;
        }

        return false;
    }

    ice_candidate_t::array_t get_local_candidates() const
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
        if (&socket == &m_sockets.front())
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

    inline void start_timer(timestamp_t timeout)
    {
        if (m_ice_params.is_full())
        {
            m_checking_timer->start(timeout);
        }
    }

    bool on_pair_state(candidate_pair_t& pair
                       , ice_state_t state
                       , bool use_candidate)
    {
        if (use_candidate)
        {
            set_active_pair(&pair);
        }
        else
        {
            if (m_active_pair == &pair)
            {
                switch(state)
                {
                    case ice_state_t::failed:
                    case ice_state_t::frozen:
                        set_active_pair(nullptr);
                        start_checking();
                    break;
                    default:;
                }
            }
            else
            {
                if (m_active_pair == nullptr)
                {
                    switch(state)
                    {
                        case ice_state_t::failed:
                        case ice_state_t::succeeded:
                        {
                            if (auto next = next_pair(&pair))
                            {
                                next->checking();
                            }
                            else
                            {
                                start_checking();
                            }
                        }
                        break;
                        default:;
                    }
                }
            }
        }
        return true;
    }

    bool on_pair_packet(candidate_pair_t& pair
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
        auto ice_params = m_ice_params;
        if (utils::property::deserialize(ice_params
                                          , params))
        {
            ice_params.local_endpoint.candidates = m_ice_params.local_endpoint.candidates;
            m_ice_params = ice_params;
            return true;

        }

        return false;
    }

    bool get_params(i_property &params) const override
    {
        auto ice_params = m_ice_params;
        ice_params.local_endpoint.candidates = get_local_candidates();
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
        return m_ice_params.local_endpoint;
    }

    ice_endpoint_t remote_endpoint() const override
    {
        return m_ice_params.remote_endpoint;
    }

    bool add_remote_candidate(const ice_candidate_t &candidate) override
    {
        return internal_add_remote_candidate(candidate);
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
