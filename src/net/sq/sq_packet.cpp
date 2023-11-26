#include "sq_packet.h"
#include "mapped_sq_header.h"

namespace mpl::net
{

sq_packet::sq_packet(const smart_buffer &fragment_data)
    : m_fragment_data(fragment_data)
{

}

sq_packet::sq_packet(smart_buffer &&fragment_data)
    : m_fragment_data(std::move(fragment_data))
{

}

void sq_packet::set_fragment_data(const smart_buffer &fragment_data)
{
    m_fragment_data = fragment_data;
}

void sq_packet::set_fragment_data(smart_buffer &&fragment_data)
{
    m_fragment_data = std::move(fragment_data);
}

smart_buffer sq_packet::fetch_fragment_data()
{
    return std::move(m_fragment_data);
}

bool sq_packet::is_valid() const
{
    if (m_fragment_data.size() >= sizeof(sq_mapped_packet_header_t)
            && m_fragment_data.size() >= header().length
            && header().is_valid())
    {
        return true;
    }

    return false;
}

sq_packet& sq_packet::detach_refs()
{
    m_fragment_data.make_unique();
    return *this;
}

void sq_packet::clear()
{
    m_fragment_data.clear();
}

const void *sq_packet::payload_data() const
{
    return static_cast<const std::uint8_t*>(m_fragment_data.data()) + sizeof(sq_mapped_packet_header_t);
}

std::size_t sq_packet::payload_size() const
{
    return header().length;
}

uint16_t sq_packet::id() const
{
    return header().id;
}

bool sq_packet::is_first() const
{
    return header().head;
}

bool sq_packet::is_last() const
{
    return header().tail;
}

bool sq_packet::is_full() const
{
    return is_first()
            && is_last();
}

const sq_mapped_packet_header_t &sq_packet::header() const
{
    return *reinterpret_cast<const sq_mapped_packet_header_t*>(m_fragment_data.data());
}

}
