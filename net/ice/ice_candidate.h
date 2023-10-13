#ifndef MPL_NET_ICE_CANDIDATE_H
#define MPL_NET_ICE_CANDIDATE_H

#include "net/socket/socket_endpoint.h"
#include "ice_types.h"

namespace mpl::net
{

struct ice_candidate_t
{
    using component_id_t = ice_component_id_t;
    constexpr static std::uint32_t local_preference = 10000;

    using array_t = std::vector<ice_candidate_t>;

    std::string                     foundation;
    component_id_t                  component_id;
    transport_id_t                  transport;
    std::uint32_t                   priority;
    socket_address_t                connection_address;
    // attributes
    ice_candidate_type_t            type;
    socket_address_t                relayed_address;
    role_t                          tcp_role;
    std::int32_t                    generation;
    std::string                     ufrag;
    std::uint32_t                   network_cost;


    struct hasher_t
    {
        std::size_t operator()(const ice_candidate_t& candidate) const;
    };

    static std::uint8_t get_type_preference(ice_candidate_type_t type);

    static ice_candidate_t build_candidate(const std::string& foundation
                                               , std::uint16_t index
                                               , std::uint8_t component_id
                                               , const socket_address_t& connection_address
                                               , transport_id_t transport_id = transport_id_t::udp
                                               , ice_candidate_type_t candidate_type = ice_candidate_type_t::host
                                               , const socket_address_t& relayed_address = {});

    static ice_candidate_t build_candidate(std::uint16_t index
                                               , std::uint8_t component_id
                                               , const socket_address_t& connection_endpoint
                                               , transport_id_t transport_id = transport_id_t::udp
                                               , ice_candidate_type_t candidate_type = ice_candidate_type_t::host
                                               , const socket_address_t& relayed_endpoint = {});

    static std::uint32_t build_priority(std::uint8_t type_preference
                                        , std::uint16_t index
                                        , std::uint8_t component_id);


    static std::uint32_t build_priority(ice_candidate_type_t type
                                        , std::uint16_t index
                                        , std::uint8_t component_id);

    static bool from_string(const std::string& param_line
                            , ice_candidate_t& ice_candidate);

    ice_candidate_t(const std::string& foundation = {}
                    , component_id_t component_id = 0
                    , transport_id_t transport = transport_id_t::undefined
                    , std::uint32_t priority = 0
                    , const socket_address_t& connection_endpoint = socket_address_t::any_v4()
                    , ice_candidate_type_t type = ice_candidate_type_t::undefined
                    , const socket_address_t& relayed_endpoint = {}
                    , role_t tcp_role = role_t::undefined
                    , std::int32_t generation = -1
                    , const std::string& ufrag = {}
                    , std::uint32_t network_cost = 0);

    bool operator == (const ice_candidate_t& other) const;
    bool operator != (const ice_candidate_t& other) const;


    bool from_string(const std::string& param_line);
    bool is_valid() const;
    std::string to_string() const;

    std::size_t hash() const;
};

}

#endif // MPL_NET_IICE_CANDIDATE_H
