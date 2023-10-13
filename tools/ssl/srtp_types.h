#ifndef SSL_SRTP_TYPES_H
#define SSL_SRTP_TYPES_H

#include <set>

namespace ssl
{

enum class srtp_profile_id_t
{
    none = 0x0000,
    aes_cm_128_hmac_sha1_80 = 0x0001,
    aes_cm_128_hmac_sha1_32 = 0x0002,
    aes_f8_128_hmac_sha1_80 = 0x0003,
    aes_f8_128_hmac_sha1_32 = 0x0004,
    aead_null_sha1_80 = 0x0005,
    aead_null_sha1_32 = 0x0006,
    aead_aes_128_gcm = 0x0007,
    aead_aes_256_gcm = 0x0008
};

using srtp_profile_id_set_t = std::set<srtp_profile_id_t>;

enum class srtp_direction_t
{
    inbound = 1,
    outbound = 2
};

enum class srtp_packet_type_t
{
    rtp,
    srtp,
    rtcp,
    srtcp
};

}

#endif // SSL_SRTP_TYPES_H
