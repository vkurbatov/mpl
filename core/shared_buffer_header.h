#ifndef MPL_SHARED_BUFFER_HEADER_H
#define MPL_SHARED_BUFFER_HEADER_H

#include "time_types.h"

namespace mpl
{

#pragma pack(push, 1)

struct shared_buffer_header_t
{
    struct state_t
    {
        std::uint32_t   packets;
        std::uint32_t   writers;
        std::uint32_t   readers;

        void reset();
    };

    std::uint64_t   position;
    timestamp_t     timestamp;
    state_t         state;

    void reset();
};

#pragma pack(pop)

}

#endif // MPL_SHARED_BUFFER_HEADER_H