#include "ice_transaction.h"

namespace mpl::net
{

ice_transaction_t::request_t::request_t(bool controlling
                                        , uint64_t tie_breaker
                                        , bool use_candidate
                                        , uint32_t priority
                                        , const std::string &username
                                        , const std::string &password)
    : controlling(controlling)
    , tie_breaker(tie_breaker)
    , use_candidate(use_candidate)
    , priority(priority)
    , username(username)
    , password(password)
{

}

ice_transaction_t::response_t::response_t(ice_result_t result
                                          , const socket_address_t &mapped_endpoint
                                          , bool xor_address
                                          , std::uint16_t error_code)
    : result(result)
    , mapped_address(mapped_endpoint)
    , xor_address(xor_address)
    , error_code(error_code)
{

}



bool ice_transaction_t::response_t::is_success() const
{
    return result == ice_result_t::success;
}


ice_transaction_t::ice_transaction_t(const socket_address_t& endpoint
                                     , const ice_transaction_t::request_t &request
                                     , const ice_transaction_t::response_t &response
                                     , uint64_t tag
                                     , timestamp_t timeout
                                     , std::size_t retries)
    : address(endpoint)
    , request(request)
    , response(response)
    , tag(tag)
    , timeout(timeout)
    , retries(retries)
{

}

}
