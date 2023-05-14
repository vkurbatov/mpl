#include "depacketizer.h"
#include "mapped_packet.h"

#include <cstring>

namespace mpl
{

depacketizer::depacketizer(i_data_object& buffer
                           , std::size_t cursor)
    : m_buffer(buffer)
    , m_cursor(cursor)
{

}

bool depacketizer::is_overload() const
{
    return m_cursor >= m_buffer.size();
}

std::size_t depacketizer::pending_size() const
{
    return m_cursor < m_buffer.size()
            ? m_buffer.size() - m_cursor
            : 0ul;
}

void depacketizer::reset()
{
    m_cursor = 0;
}

std::size_t depacketizer::cursor() const
{
    return m_cursor;
}

void depacketizer::seek(std::size_t cursor)
{
    m_cursor = std::min(cursor, m_buffer.size());
}

bool depacketizer::drop(field_type_t field_type)
{
    auto save_cursor = cursor();

    data_field_t data_field;

    if (fetch(data_field)
            && data_field.type == field_type)
    {
        return true;
    }

    seek(save_cursor);
    return false;
}

bool depacketizer::open_object()
{
    return drop(field_type_t::object_begin);
}

bool depacketizer::close_object()
{
    return drop(field_type_t::object_end);
}

bool depacketizer::fetch(data_field_t &field)
{
    if (pending_size() >= mapped_packet_t::min_header_size)
    {
        const auto ptr = static_cast<const std::uint8_t*>(m_buffer.data());
        const auto& packet = *reinterpret_cast<const mapped_packet_t*>(ptr + m_cursor);

        auto packet_size = packet.packet_size();

        if (pending_size() >= packet_size)
        {
            auto payload_size = packet.payload_size();
            auto payload = packet.payload_data();

            field.type = packet.header.field_type;
            field.size = payload_size;
            field.data = field.size == 0
                    ? nullptr
                    : payload;

            m_cursor += packet_size;

            return true;
        }
    }

    return false;
}

bool depacketizer::fetch(field_type_t field_type
                         , void *payload_data
                         , std::size_t payload_size)
{
    if (pending_size() >= mapped_packet_t::min_header_size)
    {
        const auto ptr = static_cast<const std::uint8_t*>(m_buffer.data());
        const auto& packet = *reinterpret_cast<const mapped_packet_t*>(ptr + m_cursor);
        auto packet_size = packet.packet_size();

        if (pending_size() >= packet_size)
        {
            auto read_size = packet.read_packet(field_type
                                                , payload_data
                                                , payload_size);
            if (read_size > 0)
            {
                m_cursor += packet_size;
                if (read_size < payload_size)
                {
                    std::memset(static_cast<std::uint8_t*>(payload_data) + read_size
                                , 0
                                , payload_size - read_size);
                }
                return true;
            }
        }

    }

    return false;
}

bool depacketizer::fetch_stream(void *stream_data
                                , std::size_t stream_size)
{
    if (read_stream(stream_data
                    , stream_size))
    {
        m_cursor += stream_size;
        return true;
    }

    return false;
}

bool depacketizer::read_stream(void *stream_data
                               , std::size_t stream_size)
{
    if ((m_cursor + stream_size) <= m_buffer.size())
    {
        if (stream_data != nullptr)
        {
            std::memcpy(stream_data
                        , static_cast<const std::uint8_t*>(m_buffer.data()) + m_cursor
                        , stream_size);
        }

        return true;
    }

    return false;
}

}
