#ifndef MPL_UTILS_DEPACKETIZER_H
#define MPL_UTILS_DEPACKETIZER_H

#include "core/i_data_object.h"
#include "core/i_dynamic_buffer.h"
#include "core/packet_types.h"
#include <vector>

namespace mpl
{

class depacketizer
{
    i_data_object &   m_buffer;
    std::size_t       m_cursor;
public:

    depacketizer(i_data_object& buffer
                 , std::size_t cursor = 0);

    bool is_overload() const;

    std::size_t pending_size() const;

    void reset();

    std::size_t cursor() const;
    void seek(std::size_t cursor);

    bool drop(field_type_t field_type);

    bool open_object();
    bool close_object();


    bool fetch(data_field_t& field);

    bool fetch(field_type_t field_type
               , void* payload_data = nullptr
               , std::size_t payload_size = 0);

    bool fetch_stream(void* stream_data
                      , std::size_t stream_size);

    bool read_stream(void* stream_data
                     , std::size_t stream_size);

    template<typename T>
    bool fetch_value(T& value);

    template<typename T>
    bool fetch_value(std::vector<T>& value);

    template<typename E>
    bool fetch_enum(E& value)
    {
        std::int64_t e_value = {};
        if (fetch_value(e_value))
        {
            value = static_cast<E>(e_value);
            return true;
        }
        return false;
    }
};

}

#endif // MPL_UTILS_DEPACKETIZER_H
