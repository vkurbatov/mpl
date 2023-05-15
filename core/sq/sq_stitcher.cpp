#include "sq_stitcher.h"

namespace mpl::sq
{

namespace detail
{

inline bool is_sq_ok(std::uint16_t sq_id
                     , std::uint16_t prev_sq_id)
{
    return static_cast<std::uint16_t>(prev_sq_id + 1) == sq_id;
}

inline bool is_seq_fail(std::uint16_t sq_id
                        , std::uint16_t prev_sq_id)
{
    return !is_sq_ok(sq_id
                      , prev_sq_id);
}

inline bool is_overload(std::uint16_t sq_id
                        , std::uint16_t prev_sq_id
                        , std::size_t max_jitter)
{
    return static_cast<std::uint16_t>(sq_id - prev_sq_id) > max_jitter
            && static_cast<std::uint16_t>(prev_sq_id - sq_id) > max_jitter;
}

}

sq_stitcher::sq_stitcher(const frame_handler_t &frame_handler
                         , std::size_t max_jitter)
    : m_reorder_buffer(max_jitter)
    , m_frame_handler(frame_handler)
    , m_head_id(0)
{

}

bool sq_stitcher::push_packet(sq_packet &&packet)
{
    if (packet.is_valid())
    {
        auto packet_id = packet.id();

        bool first = m_packets == 0;

        bool transit = (first || packet_id == m_head_id)
                        && packet.is_full();


        m_packets++;

        if (transit)
        {
            m_head_id = packet_id + 1;
            m_frame_handler(smart_buffer(packet.payload_data()
                                         , packet.payload_size()
                                         ));
        }
        else
        {
            if (first)
            {
                m_head_id = packet_id;
            }
            else if (detail::is_overload(packet_id
                                        , m_head_id
                                        , m_reorder_buffer.size()))
            {
                m_head_id = packet_id - m_reorder_buffer.size() + 1;
            }

            std::size_t idx = packet_id % m_reorder_buffer.size();
            m_reorder_buffer[idx] = std::move(packet.detach_refs());
        }

        process_buffer();

        return true;
    }

    return false;
}

bool sq_stitcher::push_packet(const void *packet_data, std::size_t packet_size)
{
    sq_packet packet(smart_buffer(packet_data
                                  , packet_size));

    return push_packet(std::move(packet));
}

sq_stitcher::seq_ids_t sq_stitcher::get_lost_packets() const
{
    seq_ids_t lost_packets;

    bool has_tail = false;
    auto size = m_reorder_buffer.size();
    std::uint16_t tail_d = m_head_id + size - 1;

    for (packet_id_t packet_id = tail_d
         ; packet_id != m_head_id
         ; packet_id--)
    {
        auto idx = packet_id % size;
        const auto& p = m_reorder_buffer[idx];

        if (p.is_valid()
                && p.id() == packet_id)
        {
            has_tail |= true;
            continue;
        }

        if (has_tail)
        {
            lost_packets.insert(packet_id);
        }
    }

    return lost_packets;
}

void sq_stitcher::clear_buffer()
{
    for (auto& p : m_reorder_buffer)
    {
        p.clear();
    }
}

std::size_t sq_stitcher::process_buffer()
{
    std::size_t result = 0;

    auto size = m_reorder_buffer.size();

    for (const auto& r : get_full_packet_ranges())
    {
        smart_buffer frame;

        for (auto packet_id = r.first; ; packet_id++)
        {
            auto idx = packet_id % size;
            auto& p = m_reorder_buffer[idx];


            if (p.is_full())
            {
                m_frame_handler(smart_buffer(p.payload_data()
                                             , p.payload_size()
                                             ));
                m_head_id = packet_id + 1;
                break;
            }
            else
            {
                frame.append_data(p.payload_data()
                                  , p.payload_size());

                if (p.is_last())
                {
                    m_head_id = packet_id + 1;

                    m_frame_handler(std::move(frame));
                    break;
                }
            }

            if (packet_id == r.second)
            {
                break;
            }
        }

    }

    return result;
}

void sq_stitcher::reset()
{
    clear_buffer();
    m_assembled_packet.clear();
    m_packets = 0;
}

sq_stitcher::packet_ranges_t sq_stitcher::get_full_packet_ranges() const
{
    sq_stitcher::packet_ranges_t ranges;

    auto size = m_reorder_buffer.size();
    auto packet_id = m_head_id;
    auto tail_id = packet_id + size - 1;

    packet_range_t range;
    bool found_head_packet = false;

    while(packet_id != tail_id)
    {
        packet_id_t idx = (packet_id) % size;

        const auto& p = m_reorder_buffer[idx];
        if (p.is_valid()
                && p.id() == packet_id)
        {
            if (p.is_first())
            {
                found_head_packet = true;
                range.first = packet_id;
            }

            if (found_head_packet)
            {
                if (p.is_last())
                {
                    found_head_packet = false;
                    range.second = packet_id;
                    ranges.push_back(range);
                }
            }

            packet_id++;
            continue;
        }

        break;
    }

    return ranges;
}


}
