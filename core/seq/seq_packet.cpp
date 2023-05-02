#include "seq_packet.h"
#include "mapped_seq_header.h"

namespace mpl::seq
{

seq_packet::seq_packet(const smart_buffer &fragment_data)
    : m_fragment_data(fragment_data)
{

}

seq_packet::seq_packet(smart_buffer &&fragment_data)
    : m_fragment_data(std::move(fragment_data))
{

}

void seq_packet::set_fragment_data(const smart_buffer &fragment_data)
{
    m_fragment_data = fragment_data;
}

void seq_packet::set_fragment_data(smart_buffer &&fragment_data)
{
    m_fragment_data = std::move(fragment_data);
}

smart_buffer seq_packet::fetch_fragment_data()
{
    return std::move(m_fragment_data);
}

bool seq_packet::is_valid() const
{
    if (m_fragment_data.size() >= sizeof(mapped_packet_header_t)
            && m_fragment_data.size() >= header().length
            && header().is_valid())
    {
        return true;
    }

    return false;
}

void seq_packet::detach_refs()
{
    m_fragment_data.make_store();
}

const void *seq_packet::payload_data() const
{
    return static_cast<const std::uint8_t*>(m_fragment_data.data()) + sizeof(mapped_packet_header_t);
}

std::size_t seq_packet::payload_size() const
{
    return header().length;
}

uint16_t seq_packet::id() const
{
    return header().id;
}

bool seq_packet::is_first() const
{
    return header().head;
}

bool seq_packet::is_last() const
{
    return header().tail;
}

const mapped_packet_header_t &seq_packet::header() const
{
    return *reinterpret_cast<const mapped_packet_header_t*>(m_fragment_data.data());
}

}
