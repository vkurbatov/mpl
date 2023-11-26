#ifndef MPL_NET_STUN_ERROR_CODES_H
#define MPL_NET_STUN_ERROR_CODES_H

#include <cstdint>

namespace mpl::net
{

using stun_error_code_t = std::uint16_t;

constexpr stun_error_code_t error_try_alternate                     = 300;

constexpr stun_error_code_t error_bad_request                       = 400;
constexpr stun_error_code_t error_unautorized                       = 401;
                                                                    // 402
constexpr stun_error_code_t error_forbidden                         = 403;
                                                                    // 404
constexpr stun_error_code_t error_mobility_forbidden                = 405;
                                                                    // 406-419
constexpr stun_error_code_t error_unknown_attribute                 = 420;
                                                                    // 421-436
constexpr stun_error_code_t error_allocation_mismatch               = 437;
constexpr stun_error_code_t error_stale_nonce                       = 438;
                                                                    // 439
constexpr stun_error_code_t error_address_family_not_supported      = 440;
constexpr stun_error_code_t error_wrong_credentials                 = 441;
constexpr stun_error_code_t error_unsupported_transport_protocol    = 442;
constexpr stun_error_code_t error_peer_address_family_mismatch      = 443;
                                                                    // 444-445
constexpr stun_error_code_t error_connection_already_exists         = 446;
constexpr stun_error_code_t error_connection_timeoit_of_failure     = 447;
                                                                    // 448-485
constexpr stun_error_code_t error_allocation_quota_roached          = 486;
constexpr stun_error_code_t error_role_conflict                     = 487;
                                                                    // 488-499
constexpr stun_error_code_t error_server_error                      = 500;
                                                                    // 501-507
constexpr stun_error_code_t error_insufficient_capacity             = 508;
}

#endif // MPL_NET_STUN_ERROR_CODES_H
