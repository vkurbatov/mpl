#ifndef MPL_SQ_STITCHER_H
#define MPL_SQ_STITCHER_H

#include "sq_packet.h"

#include <functional>
#include <set>

namespace mpl::sq
{

class sq_stitcher
{
public:
    static constexpr std::size_t default_max_jitter = 50;

    using seq_ids_t = std::set<packet_id_t>;
    using frame_handler_t = std::function<void(smart_buffer&&)>;
private:

    using packet_range_t = std::pair<packet_id_t, packet_id_t>;
    using packet_ranges_t = std::vector<packet_range_t>;

    sq_packet::array_t      m_reorder_buffer;
    frame_handler_t         m_frame_handler;
    smart_buffer            m_assembled_packet;

    std::uint16_t           m_head_id;
    std::size_t             m_packets;

public:
    sq_stitcher(const frame_handler_t& frame_handler
                , std::size_t max_jitter = default_max_jitter);

    bool push_packet(sq_packet&& packet);
    bool push_packet(const void* packet_data, std::size_t packet_size);

    seq_ids_t get_lost_packets() const;

    void clear_buffer();
    std::size_t process_buffer();

    void reset();
private:
    packet_ranges_t get_full_packet_ranges() const;
};

}

#endif // MPL_SQ_STITCHER_H
