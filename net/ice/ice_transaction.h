#ifndef MPL_NET_ICE_TRANSACTION_H
#define MPL_NET_ICE_TRANSACTION_H

#include "core/time_types.h"
#include "net/socket/socket_endpoint.h"
#include "net/stun/stun_types.h"

#include <string>
#include <cstdint>

namespace mpl::net
{

struct ice_transaction_t
{    
    enum class ice_result_t
    {
        processed,
        success,
        timeout,
        failed,
        canceled
    };

    struct request_t
    {
        bool                controlling;
        std::uint64_t       tie_breaker;
        bool                use_candidate;
        std::uint32_t       priority;
        std::string         username;
        std::string         password;

        request_t(bool controlling = false
                  , std::uint64_t tie_breaker = 0
                  , bool use_candidate = false
                  , std::uint32_t priority = 0
                  , const std::string& username = {}
                  , const std::string& password = {});
    };

    struct response_t
    {
        ice_result_t        result;
        socket_address_t    mapped_address;
        bool                xor_address;
        std::uint16_t       error_code;

        response_t(ice_result_t result = ice_result_t::processed
                   , const socket_address_t& mapped_address = {}
                   , bool xor_address = false
                   , std::uint16_t error_code = 0);

        bool is_success() const;
    };


    socket_address_t        address;
    request_t               request;
    response_t              response;
    std::uint64_t           tag;
    timestamp_t             timeout;
    std::size_t             retries;

    ice_transaction_t(const socket_address_t& address = {}
                      , const request_t& request = {}
                      , const response_t& response = {}
                      , std::uint64_t tag = 0
                      , timestamp_t timeout = timestamp_infinite
                      , std::size_t retries = 0);
};

}

#endif // MPL_NET_ICE_TRANSACTION_H
