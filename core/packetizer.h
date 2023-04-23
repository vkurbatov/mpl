#ifndef MPL_PACKETIZER_H
#define MPL_PACKETIZER_H

#include "i_fifo_buffer_writer.h"
#include "packet_types.h"

namespace mpl
{

class packetizer
{
    i_fifo_buffer_writer&   m_writer;
public:
    packetizer(i_fifo_buffer_writer& writer);

    bool add(field_type_t field_type
             , const void* payload_data = nullptr
             , std::size_t payload_size = 0);

    bool add_stream(const void* stream_data = nullptr
                    , std::size_t stream_size = 0);

    template<typename T>
    bool add_value(const T& value);
};

}

#endif // MPL_PACKETIZER_H
