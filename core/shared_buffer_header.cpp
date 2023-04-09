#include "shared_buffer_header.h"

namespace mpl
{

void shared_buffer_header_t::state_t::reset()
{
    packets = 0;
    writers = 0;
    readers = 0;
}

void shared_buffer_header_t::reset()
{
    position = 0;
    timestamp = 0;
    state.reset();
}

}
