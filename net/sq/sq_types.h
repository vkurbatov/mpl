#ifndef MPL_NET_SEQ_TYPES_H
#define MPL_NET_SEQ_TYPES_H

#include "core/time_types.h"

namespace mpl::net
{

constexpr std::uint16_t default_sq_fragment_size = 1500;
constexpr std::uint16_t defalt_sq_fragment_buffer_size = 1000;
constexpr timestamp_t default_sq_nack_request_period = durations::milliseconds(100);


using packet_id_t = std::uint16_t;
enum class sq_packet_type_t
{
    undefined = 0,
    fragment = 1,
    request_nack = 2,
    reserved3,
    reserved4,
    reserved5,
    reserved6,
    reserved7
};


}

#endif // MPL_NET_SEQ_TYPES_H
