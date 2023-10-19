#include "ice_candidate.h"

#include "utils/enum_utils.h"
#include "utils/common_utils.h"
#include "utils/convert_utils.h"

template<>
struct std::hash<mpl::net::ice_candidate_t>
{
    std::size_t operator()(const mpl::net::ice_candidate_t& s) const noexcept
    {
        return s.hash();
    }
};

namespace mpl::net
{

namespace detail
{

inline bool is_valid_ice_transport(transport_id_t transport_id)
{
    switch(transport_id)
    {
        case transport_id_t::udp:
        case transport_id_t::tcp:
            return true;
        break;
    }

    return false;
}

}

std::uint8_t ice_candidate_t::get_type_preference(ice_candidate_type_t type)
{
    static const std::uint8_t type_preference_table [] =
    {
        0
        , 126
        , 110
        , 100
        , 0
    };

    return type_preference_table[static_cast<std::int32_t>(type)];
}

const ice_candidate_t *ice_candidate_t::find(const array_t &candidates
                                             , const socket_address_t &address
                                             , transport_id_t transport_id)
{
    if (auto it = std::find_if(candidates.begin()
                               , candidates.end()
                               , [&address, transport_id](const ice_candidate_t& c) { return c.connection_address == address && c.transport == transport_id; } )
        ; it != candidates.end())
    {
        return &(*it);
    }

    return nullptr;
}

ice_candidate_t *ice_candidate_t::find(array_t &candidates
                                       , const socket_address_t &address
                                       , transport_id_t transport_id)
{
    if (auto it = std::find_if(candidates.begin()
                               , candidates.end()
                               , [&address, transport_id](const ice_candidate_t& c) { return c.connection_address == address && c.transport == transport_id; } )
        ; it != candidates.end())
    {
        return &(*it);
    }

    return nullptr;
}

ice_candidate_t ice_candidate_t::build_candidate(const std::string& foundation
                                                         , std::uint16_t index
                                                         , uint8_t component_id
                                                         , const socket_address_t& connection_endpoint
                                                         , transport_id_t transport_id
                                                         , ice_candidate_type_t candidate_type
                                                         , const socket_address_t &relayed_endpoint)
{
    return  { foundation
                , component_id
                , transport_id
                , ice_candidate_t::build_priority(candidate_type
                                                      , index
                                                      , component_id)
                , connection_endpoint
                , candidate_type
                , relayed_endpoint };
}

ice_candidate_t ice_candidate_t::build_candidate(std::uint16_t index
                                                         , std::uint8_t component_id
                                                         , const socket_address_t &connection_endpoint
                                                         , transport_id_t transport_id
                                                         , ice_candidate_type_t candidate_type
                                                         , const socket_address_t &relayed_endpoint)
{
    return build_candidate(std::to_string(index)
                           , index
                           , component_id
                           , connection_endpoint
                           , transport_id
                           , candidate_type
                           , relayed_endpoint);
}

std::uint32_t ice_candidate_t::build_priority(std::uint8_t type_preference
                                                  , std::uint16_t index
                                                  , std::uint8_t component_id)
{
     return static_cast<std::uint32_t>(type_preference) << 24
             | static_cast<std::uint32_t>(local_preference - index) << 8
             | static_cast<std::uint32_t>(255 - component_id);
}

std::uint32_t ice_candidate_t::build_priority(ice_candidate_type_t type
                                                  , std::uint16_t index
                                                  , std::uint8_t component_id)
{
    return build_priority(get_type_preference(type)
                          , index
                          , component_id);
}

bool ice_candidate_t::from_string(const std::string &param_line
                                  , ice_candidate_t &ice_candidate)
{
    if (!param_line.empty())
    {
        auto params = utils::split_lines(param_line, ' ');
        if (params.size() >= 6)
        {
            if (params[0].find("candidate:") == 0)
            {
                params[0] = params[0].substr(10);
            }

            if (utils::convert(params[0], ice_candidate.foundation)
                    && utils::convert(params[1], ice_candidate.component_id)
                    && utils::convert(params[2], ice_candidate.transport)
                    && utils::convert(params[3], ice_candidate.priority)
                    && utils::convert(params[5], ice_candidate.connection_address.port))
            {
                ice_candidate.connection_address.address.from_string(params[4]);
                if (ice_candidate.is_valid())
                {
                    if (params.size() % 2 == 0)
                    {
                        for (std::size_t i = 6; i < params.size(); i += 2)
                        {
                            const auto& key = params[i];
                            const auto& value = params[i + 1];

                            if (key == "typ")
                            {
                                ice_candidate.type = utils::string_to_enum(value, ice_candidate_type_t::undefined);
                            }
                            else if (key == "raddr")
                            {
                                ice_candidate.relayed_address.address = ip_address_t(value);
                            }
                            else if (key == "rport")
                            {
                                utils::convert(value
                                               , ice_candidate.relayed_address.port);
                            }
                            else if (key == "tcptype")
                            {
                                ice_candidate.tcp_role = utils::string_to_enum(value, role_t::undefined);
                            }
                            else if (key == "generation")
                            {
                                utils::convert(value
                                               , ice_candidate.generation);
                            }
                            else if (key == "ufrag")
                            {
                                ice_candidate.ufrag = value;
                            }
                            else if (key == "network-cost")
                            {
                                utils::convert(value
                                               , ice_candidate.network_cost);
                            }
                        }
                    }
                    return true;
                }
            }
        }

    }

    return false;
}

ice_candidate_t::ice_candidate_t(const std::string& foundation
                                 , component_id_t component_id
                                 , transport_id_t transport
                                 , uint32_t priority
                                 , const socket_address_t& connection_address
                                 , ice_candidate_type_t type
                                 , const socket_address_t& relayed_address
                                 , role_t tcp_role
                                 , std::int32_t generation
                                 , const std::string& ufrag
                                 , std::uint32_t network_cost)
    : foundation(foundation)
    , component_id(component_id)
    , transport(transport)
    , priority(priority)
    , connection_address(connection_address)
    , type(type)
    , relayed_address(relayed_address)
    , tcp_role(tcp_role)
    , generation(generation)
    , ufrag(ufrag)
    , network_cost(network_cost)

{

}

bool ice_candidate_t::operator ==(const ice_candidate_t &other) const
{
    return foundation == other.foundation
            && component_id == other.component_id
            && transport == other.transport
            && priority == other.priority
            && connection_address == other.connection_address
            && type == other.type
            && relayed_address == other.relayed_address
            && tcp_role == other.tcp_role
            && generation == other.generation
            && ufrag == other.ufrag
            && network_cost == other.network_cost;
}

bool ice_candidate_t::operator !=(const ice_candidate_t &other) const
{
    return !operator == (other);
}

bool ice_candidate_t::from_string(const std::string &param_line)
{
    return from_string(param_line
                       , *this);
}

bool ice_candidate_t::is_valid() const
{
    return !foundation.empty()
            // && component_id > 0 && component_id < 3
            && detail::is_valid_ice_transport(transport)
            && connection_address.is_defined();
}

std::string ice_candidate_t::to_string() const
{
    std::string result;
    result.append(foundation)
            .append(" ").append(std::to_string(component_id))
            .append(" ").append(utils::to_upper(utils::enum_to_string(transport)))
            .append(" ").append(std::to_string(priority))
            .append(" ").append(connection_address.address.to_string())
            .append(" ").append(std::to_string(connection_address.port));

    // attributes
    if (type != ice_candidate_type_t::undefined)
    {
        result.append(" typ ").append(utils::enum_to_string(type));

        if (type != ice_candidate_type_t::host
                && relayed_address.is_defined())
        {
            result.append(" raddr ").append(relayed_address.address.to_string());
            result.append(" rport ").append(std::to_string(relayed_address.port));
        }
    }

    if (transport == transport_id_t::tcp)
    {
        result.append(" tcptype ").append(utils::enum_to_string(tcp_role));
    }

    if (generation >= 0)
    {
        result.append(" generation ").append(std::to_string(generation));
    }

    if (!ufrag.empty())
    {
        result.append(" ufrag ").append(ufrag);
    }

    if (network_cost > 0)
    {
        result.append(" network-cost ").append(std::to_string(network_cost));
    }

    return result;
}

bool ice_candidate_t::is_equal_endpoint(const ice_candidate_t &other) const
{
    return transport == other.transport
            && connection_address == other.connection_address;
}

std::size_t ice_candidate_t::hash() const
{
    return std::hash<std::string>()(foundation)
            ^ std::hash<std::uint8_t>()(component_id)
            ^ std::hash<std::int32_t>()(static_cast<std::int32_t>(transport))
            ^ std::hash<std::uint16_t>()(priority)
            ^ connection_address.hash()
            ^ std::hash<std::int32_t>()(static_cast<std::int32_t>(type))
            ^ relayed_address.hash();
}


}
