#ifndef MPL_SEQ_PACKET_BUILDER_H
#define MPL_SEQ_PACKET_BUILDER_H

#include "core/smart_buffer.h"
#include <set>

namespace mpl::sq
{

struct sq_packet_builder_t
{
    std::uint8_t    session_id;
    std::uint16_t   packet_id;
    std::uint32_t   max_fragment_size;
    std::size_t     max_nack_group_size;

    sq_packet_builder_t(std::uint8_t session_id = 0
                         , std::uint16_t packet_id = 0
                         , std::uint32_t max_fragment_size = 0
                         , std::size_t max_nack_group_size = 10);

    smart_buffer::array_t build_fragments(const void* data
                                            , std::size_t size);

    smart_buffer::array_t build_nack_request(const std::set<std::uint16_t>& nack_ids);
};

}

#endif // MPL_SEQ_PACKET_BUILDER_H
