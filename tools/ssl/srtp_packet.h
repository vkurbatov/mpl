#ifndef SSL_SRTP_PACKET_H
#define SSL_SRTP_PACKET_H

#include "srtp_types.h"
#include <cstdint>

namespace pt::ssl
{

struct srtp_packet_t
{
    srtp_packet_type_t  type;
    const void*         data;
    std::size_t         size;

    srtp_packet_t(srtp_packet_type_t type = srtp_packet_type_t::rtp
                  , const void* data = nullptr
                  , std::size_t size = 0);

    bool is_valid() const;
    bool is_encrypted() const;
    bool is_rtcp() const;
};

}

#endif // SSL_SRTP_PACKET_H
