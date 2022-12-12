#include "visca_base.h"
#include <cstring>

namespace visca
{

response_packet_t::response_packet_t(std::uint8_t address
                                    , response_type_t type
                                    , packet_data_t&& response_data)
    : address(address)
    , type(type)
    , response_data(std::move(response_data))
{

}

void response_packet_t::clear()
{
    type = response_type_t::unknown;
    address = 0;
    response_data.clear();
}

response_parser_t::response_parser_t()
    : parse_state(parse_state_t::header)
{

}

packet_queue_t response_parser_t::push_data(const void *data
                                  , std::size_t size)
{
    packet_queue_t response_queue;

    auto data_ptr = static_cast<const std::uint8_t*>(data);
    auto data_end = data_ptr + size;

    while (data_ptr < data_end)
    {
        switch(parse_state)
        {
            case parse_state_t::header:
            {
                visca_header_t header{};
                header.header = *data_ptr;

                if (header.start == 1
                        && header.dst_addr == 0
                        && (header.broadcast == 1
                            ? header.src_addr == 0
                            : header.src_addr != 0))
                {
                    current_packet.address = header.src_addr;
                    parse_state = parse_state_t::response_type;
                }
            }
            break;
            case parse_state_t::response_type:
            {
                auto type = *data_ptr >> 4;

                if (type >= static_cast<std::uint8_t>(response_type_t::address)
                    && type <= static_cast<std::uint8_t>(response_type_t::error))
                {
                    current_packet.type = static_cast<response_type_t>(type);
                    parse_state = parse_state_t::terminator;
                }
                else
                {
                    reset();
                }
            }
            break;

            case parse_state_t::terminator:
            {
                if (*data_ptr != visca_eof)
                {
                    current_packet.response_data.push_back(*data_ptr);
                }
                else
                {
                    response_queue.push(current_packet);
                    reset();
                }
            }
            break;
        }
        data_ptr++;
    }

    return response_queue;
}

void response_parser_t::reset()
{
    parse_state = parse_state_t::header;
    current_packet.clear();

}

visca_config_t::visca_config_t(uint32_t reply_timeout
                               , uint8_t pan_speed
                               , uint8_t tilt_speed)
    : reply_timeout(reply_timeout)
    , pan_speed(pan_speed)
    , tilt_speed(tilt_speed)
{

}

}
