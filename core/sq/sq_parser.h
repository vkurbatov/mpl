#ifndef MPL_SEQ_PARSER_H
#define MPL_SEQ_PARSER_H

#include "core/smart_buffer.h"
#include "sq_packet.h"
#include <functional>

namespace mpl::sq
{

class sq_parser
{
public:
    using packet_handler_t = std::function<void(sq_packet&&)>;
private:

    smart_buffer            m_packet_buffer;
    packet_handler_t        m_packet_handler;

public:

    sq_parser(const packet_handler_t& packet_handler);

    void push_stream(const void* stream_data
                     , std::size_t stream_size);

    void push_stream(const smart_buffer& stream);

    void reset();
};

}

#endif // MPL_SEQ_PARSER_H
