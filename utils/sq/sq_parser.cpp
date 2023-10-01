#include "sq_parser.h"
#include "mapped_sq_header.h"

namespace mpl::sq
{

sq_parser::sq_parser(const packet_handler_t& packet_handler)
    : m_packet_handler(packet_handler)
{

}

void sq_parser::push_stream(const void *stream_data
                             , std::size_t stream_size)
{
    push_stream(smart_buffer(stream_data
                             , stream_size));
}

void sq_parser::push_stream(const smart_buffer &stream)
{
    auto ptr = static_cast<const std::uint8_t*>(stream.data());
    auto size = stream.size();

    while(size > 0)
    {
        std::size_t processed_size = 1;
        if (m_packet_buffer.is_empty())
        {
            if (*ptr == fragment_signature)
            {
                bool handled = false;
                if (size >= sizeof(mapped_packet_header_t))
                {
                    const mapped_packet_header_t& header = *reinterpret_cast<const mapped_packet_header_t*>(ptr);
                    if (header.is_valid())
                    {
                        if (header.packet_size() <= size)
                        {
                            processed_size = std::min(header.packet_size(), size);
                            smart_buffer packet_buffer(ptr, processed_size);
                            sq_packet packet(std::move(packet_buffer));
                            if (packet.is_valid())
                            {
                                handled = true;
                                m_packet_handler(std::move(packet));
                            }
                        }
                    }
                }

                if (!handled)
                {
                    processed_size = size;
                    m_packet_buffer.append_data(ptr
                                                , size);
                }
            }
        }
        else
        {
            bool packet_completed = false;
            if (m_packet_buffer.size() >= sizeof(mapped_packet_header_t))
            {
                const mapped_packet_header_t& header = *reinterpret_cast<const mapped_packet_header_t*>(m_packet_buffer.data());
                if (header.is_valid())
                {
                    auto need_size = header.packet_size() - m_packet_buffer.size();
                    processed_size = std::min(need_size, size);
                    packet_completed = need_size == processed_size;
                }
            }
            else
            {
                auto need_size = sizeof(mapped_packet_header_t) - m_packet_buffer.size();
                processed_size = std::min(need_size, size);
            }

            if (processed_size > 0)
            {
                m_packet_buffer.append_data(ptr
                                            , processed_size);

                if (packet_completed)
                {
                    sq_packet packet(std::move(m_packet_buffer));
                    if (packet.is_valid())
                    {
                        m_packet_handler(std::move(packet));
                    }
                }
            }
            else
            {
                m_packet_buffer.clear();
            }
        }

        ptr += processed_size;
        size -= processed_size;
    }
}

void sq_parser::reset()
{
    m_packet_buffer.clear();
}

}
