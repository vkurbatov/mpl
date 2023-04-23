#include "packetizer.h"
#include "common_types.h"
#include "mapped_packet.h"

namespace mpl
{

#define __declare_

namespace detail
{
    void append_data(raw_array_t& packet, const void* data, std::size_t size)
    {
        if (data != nullptr && size > 0)
        {
            packet.insert(packet.end()
                          , static_cast<const typename raw_array_t::value_type*>(data)
                          , static_cast<const typename raw_array_t::value_type*>(data) + size);
        }
    }

    mapped_packet_t& create_packet_header(field_type_t type, std::size_t payload_size)
    {
        static thread_local raw_array_t header_data(mapped_packet_t::max_header_size);
        mapped_packet_t& packet = *reinterpret_cast<mapped_packet_t*>(header_data.data());

        packet.write_packet(type
                            , nullptr
                            , payload_size);
        return packet;
    }
}

packetizer::packetizer(i_fifo_buffer_writer &writer)
    : m_writer(writer)
{

}

bool packetizer::add(field_type_t field_type
                     , const void *payload_data
                     , std::size_t payload_size)
{
    auto& packet = detail::create_packet_header(field_type
                                                , payload_size);

    if (add_stream(&packet, packet.payload_size()))
    {
        return payload_data == nullptr || add_stream(payload_data
                                                     , payload_size);
    }

    return false;
}

bool packetizer::add_stream(const void *stream_data
                            , std::size_t stream_size)
{
    return m_writer.push_data(stream_data
                              , stream_size);
}

template<typename T>
bool packetizer::add_value(const T &value)
{
    return false;
}



}
