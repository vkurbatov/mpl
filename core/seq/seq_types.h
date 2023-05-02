#ifndef MPL_SEQ_TYPES_H
#define MPL_SEQ_TYPES_H

#include "core/time_types.h"

namespace mpl::seq
{

constexpr std::uint16_t default_seq_fragment_size = 1400;
constexpr std::uint16_t defalt_seq_fragment_buffer_size = 1000;
constexpr timestamp_t default_nack_request_period = durations::milliseconds(100);

enum class packet_type_t
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

#endif // MPL_SEQ_TYPES_H
