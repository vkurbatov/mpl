#ifndef SSL_MAPPED_DTLS_HEADER_H
#define SSL_MAPPED_DTLS_HEADER_H

#include "ssl_types.h"
#include <cstdint>

namespace ssl
{

#pragma pack(push, 1)

struct mapped_handshake_header_t
{
    std::uint8_t    type;
    std::uint8_t    length[3];
};

struct mapped_dtls_header_t
{
    std::uint8_t    content_type;
    std::uint16_t   version;
    std::uint16_t   epoch;
    std::uint8_t    sequence_number[6];
    std::uint16_t   length;

    ssl_content_type_t get_content_type() const;
    ssl_version_t get_version() const;
    std::uint64_t get_seq_number() const;
    std::uint16_t get_lenght() const;

    bool is_valid() const;

    const mapped_handshake_header_t* get_handshake_header() const;
};

#pragma pack(pop)

}

#endif // SSL_MMAPPED_DTLS_HEADER_H
