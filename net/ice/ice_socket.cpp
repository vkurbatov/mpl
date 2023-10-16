#include "ice_socket.h"
#include "core/i_message_event.h"
#include "core/event_channel_state.h"
#include "core/i_message_source.h"

#include "net/socket/i_socket_packet.h"
#include "net/stun/stun_packet_impl.h"
#include "net/net_message_types.h"
#include "net/net_utils.h"

#include "tools/io/net/net_utils.h"

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

socket_address_t::array_t get_stun_server_addresses(const ice_server_params_t::array_t& ice_servers)
{
    socket_address_t::array_t ice_address_array;

    for (const auto& dns_name : ice_server_params_t::get_dns_names(ice_servers))
    {
        socket_address_t address(dns_name);
        if (address.is_valid())
        {
            if (address.port == port_any)
            {
                address.port = port_stun;
            }

            ice_address_array.emplace_back(address);
        }
    }

    return ice_address_array;
}

}

ice_socket_basic::ice_socket_basic(i_socket_transport::u_ptr_t &&socket
                                   , i_timer_manager &timer_manager
                                   , ice_component_id_t component_id)
    : m_timer_manager(timer_manager)
    , m_socket(std::move(socket))
    , m_component_id(component_id)
    , m_socket_sink([&](auto&& message) { return on_socket_mesage(message); })
    , m_ice_controller(*this
                       , m_timer_manager)
    , m_gathering_timer(m_timer_manager.create_timer([&]{ on_gathering_timeout(); }))
    , m_established(false)
{
    m_socket->source(0)->add_sink(&m_socket_sink);
}

ice_socket_basic::~ice_socket_basic()
{
    m_ice_controller.reset();
    m_gathering_timer.reset();
    set_gathering_state(ice_gathering_state_t::closed);
    m_socket->control(channel_control_t::close());
    m_socket->source(0)->remove_sink(&m_socket_sink);
    m_socket.reset();
}

bool ice_socket_basic::control(const channel_control_t &control)
{
    return m_socket->control(control);
}

bool ice_socket_basic::start_gathering(const ice_server_params_t::array_t &stun_servers
                                       , timestamp_t timeout)
{
    if (is_established())
    {
        set_gathering_state(ice_gathering_state_t::gathering);
        auto stun_addresses = detail::get_stun_server_addresses(stun_servers);
        if (!stun_addresses.empty())
        {
            bool result = false;
            for (const auto& a : stun_addresses)
            {
                ice_transaction_t transaction;
                transaction.tag = ice_gathering_id;
                transaction.address = a;
                result = send_transaction(std::move(transaction)
                                            , false);
            }
        }

        set_gathering_state(ice_gathering_state_t::completed);
        return true;
    }

    return false;
}

bool ice_socket_basic::stop_gathering()
{
    return true;
}

bool ice_socket_basic::is_open() const
{
    return m_socket->is_open();
}

bool ice_socket_basic::is_established() const
{
    return m_established;
}

channel_state_t ice_socket_basic::channel_state() const
{
    return m_socket->state();
}

ice_gathering_state_t ice_socket_basic::gathering_state() const
{
    return m_gathering_state;
}

bool ice_socket_basic::send_transaction(ice_transaction_t &&transaction
                                        , bool flush)
{
    if (flush)
    {
        m_ice_controller.reset(transaction.tag);
    }

    return m_ice_controller.send_request(std::move(transaction));
}

ice_candidate_t ice_socket_basic::create_candidate(const socket_address_t &address
                                                   , ice_candidate_type_t type)
{
    auto candidate = ice_candidate_t::build_candidate(query_foundation()
                                                      , m_candidates.size()
                                                      , m_component_id
                                                      , address
                                                      , m_socket->transport_id()
                                                      , type);
    if (type != ice_candidate_type_t::host)
    {
        candidate.relayed_address = m_socket->local_endpoint().socket_address;
    }

    return candidate;
}

void ice_socket_basic::set_gathering_state(ice_gathering_state_t new_state
                                           , const std::string_view &reason)
{
    if (m_gathering_state != new_state)
    {
        m_gathering_state = new_state;
        on_gathering_state(new_state
                           , reason);
    }
}

void ice_socket_basic::set_established(bool established)
{
    if (m_established != established)
    {
        m_established = established;
        on_established(established);
    }
}

bool ice_socket_basic::on_socket_mesage(const i_message &message)
{
    switch(message.category())
    {
        case message_category_t::event:
        {
            auto& event = static_cast<const i_message_event&>(message).event();
            if (message.subclass() == message_core_class
                    && event.event_id == event_channel_state_t::id)
            {
                on_socket_state(static_cast<const event_channel_state_t&>(event).state
                                , static_cast<const event_channel_state_t&>(event).reason);
                return true;
            }
        }
        break;
        case message_category_t::packet:
        {
            if (message.subclass() == message_net_class)
            {
                auto& net_packet = static_cast<const i_net_packet&>(message);
                switch(net_packet.transport_id())
                {
                    case transport_id_t::udp:
                    case transport_id_t::tcp:
                    {
                        auto& socket_packet = static_cast<const i_socket_packet&>(net_packet);
                        switch(utils::parse_protocol(socket_packet.data()
                                                     , socket_packet.size()))
                        {
                            case protocol_type_t::stun:
                            {
                                stun_packet_impl stun_packet(smart_buffer(&socket_packet)
                                                             , socket_packet.endpoint().socket_address);
                                if (stun_packet.is_valid())
                                {
                                    return m_ice_controller.push_packet(stun_packet);
                                }
                            }
                            break;
                            default:;
                        }

                        return on_socket_packet(socket_packet);

                    }
                    break;
                    default:;
                }
            }
        }
        break;
        default:;
    }

    return false;
}

void ice_socket_basic::on_mapped_address(const socket_address_t &address)
{
    if (ice_candidate_t::find(m_candidates
                              , address
                              , m_socket->transport_id()) == nullptr)
    {
        add_candidate(create_candidate(address
                                       , ice_candidate_type_t::host));
    }
}

void ice_socket_basic::check_mapped_address(const socket_address_t &address)
{
    if (m_mapped_address != address)
    {
        m_mapped_address = address;
        on_mapped_address(address);
    }
}

bool ice_socket_basic::add_candidate(ice_candidate_t&& candidate)
{
    if (on_candidate(candidate))
    {
        m_candidates.emplace_back(std::move(candidate));
        return true;
    }

    return false;
}

bool ice_socket_basic::on_send_packet(const i_stun_packet &stun_packet)
{
    if (auto sink = m_socket->sink(0))
    {
        return sink->send_message(stun_packet);
    }

    return false;
}

std::string ice_socket_basic::on_autorized(const ice_transaction_t &transaction)
{
    return get_password();
}

bool ice_socket_basic::on_request(ice_transaction_t &transaction)
{
    return on_ice_request(std::move(transaction));
}

bool ice_socket_basic::on_response(ice_transaction_t &transaction)
{
    return on_ice_response(std::move(transaction));
}

std::string ice_socket_basic::query_foundation()
{
    return std::to_string(m_candidates.size());
}

std::string ice_socket_basic::get_password() const
{
    return {};
}

void ice_socket_basic::on_socket_state(channel_state_t /*new_state*/
                                       , const std::string_view &/*reason*/)
{

}

void ice_socket_basic::on_gathering_state(ice_gathering_state_t /*new_state*/
                                          , const std::string_view &/*reason*/)
{

}

bool ice_socket_basic::on_socket_packet(const i_socket_packet &/*socket_packet*/)
{
    return false;
}

bool ice_socket_basic::on_candidate(const ice_candidate_t &/*candidate*/)
{
    return false;
}

void ice_socket_basic::on_established(bool /*established*/)
{

}


}
