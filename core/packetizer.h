#ifndef MPL_PACKETIZER_H
#define MPL_PACKETIZER_H

#include "packet_types.h"
#include "i_dynamic_buffer.h"
#include <vector>

namespace mpl
{

class packetizer
{
    i_dynamic_buffer&       m_buffer;
public:
    packetizer(i_dynamic_buffer& buffer);

    bool open_object();
    bool close_object();

    bool add(field_type_t field_type
             , const void* payload_data = nullptr
             , std::size_t payload_size = 0);

    bool add_stream(const void* stream_data = nullptr
                    , std::size_t stream_size = 0);

    template<typename T>
    bool add_value(const T& value);

    template<typename T>
    bool add_value(const std::vector<T>& value);

    template<typename E>
    bool add_enum(const E& value)
    {
        return add_value(static_cast<std::int64_t>(value));
    }
};

}

#endif // MPL_PACKETIZER_H
