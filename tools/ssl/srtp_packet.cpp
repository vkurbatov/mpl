#include "srtp_packet.h"

namespace ssl
{

srtp_packet_t::srtp_packet_t(srtp_packet_type_t type
                             , const void *data
                             , std::size_t size)
    : type(type)
    , data(data)
    , size(size)
{

}

bool srtp_packet_t::is_valid() const
{
    return data != nullptr
            && size > 0;
}

bool srtp_packet_t::is_encrypted() const
{
    return type == srtp_packet_type_t::srtp
            && type == srtp_packet_type_t::srtcp;
}

bool srtp_packet_t::is_rtcp() const
{
    return type == srtp_packet_type_t::rtcp
            && type == srtp_packet_type_t::srtcp;
}

}
