#ifndef MPL_NET_STUN_TYPES_H
#define MPL_NET_STUN_TYPES_H

#include <cstdint>
#include <array>

namespace mpl::net
{

constexpr std::uint32_t stun_message_cookie = 0x42a41221;
constexpr std::uint32_t stun_fingerprint_xor_value = 0x5354554e;

constexpr std::size_t stun_transaction_data_size = 12;
constexpr std::size_t stun_header_size = 8 + stun_transaction_data_size;
constexpr std::size_t stun_attribute_header_size = 4;
constexpr std::size_t stun_align_size = 4;

using stun_transaction_id_t = std::array<std::uint8_t, stun_transaction_data_size>;

enum class stun_message_class_t
{
    undefined           = -1,
    request             = 0,
    indication          = 1,
    success_response    = 2,
    error_response      = 3
};

enum class stun_method_t
{
    undefined           = -1,
    binding             = 1
};

enum class stun_attribute_id_t
{
    undefined           = -1,
    mapped_address      = 0x0001,
    username            = 0x0006,
    message_integrity   = 0x0008,
    error_code          = 0x0009,
    unknown             = 0x000a,
    realm               = 0x0014,
    nonce               = 0x0015,
    xor_mapped_address  = 0x0020,
    priority            = 0x0024,
    use_candidate       = 0x0025,
    software            = 0x8022,
    alternate_server    = 0x8023,
    fingerprint         = 0x8028,
    ice_controlled      = 0x8029,
    ice_controlling     = 0x802a
};

enum class stun_authentification_result_t
{
    undefined           = -1,
    ok                  = 0,
    unautorized         = 1,
    bad_request         = 2
};

enum class stun_protocol_family_t
{
    undefined = -1,
    ip4,
    ip6
};

}

#endif // MPL_NET_STUN_TYPES_H
