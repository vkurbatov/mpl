#include "mapped_sq_header.h"

namespace mpl::net
{

sq_mapped_packet_header_t::sq_mapped_packet_header_t(sq_packet_type_t packet_type
                                               , uint16_t id
                                               , uint32_t length)
    : signature(sq_fragment_signature)
    , padding(sq_fragment_header_padding)
    , id(id)
    , length(length)
{
    set_packet_type(packet_type);
}

void sq_mapped_packet_header_t::set_packet_type(sq_packet_type_t packet_type)
{
    if (packet_type != sq_packet_type_t::undefined)
    {
        this->packet_type = static_cast<std::uint8_t>(packet_type);
    }
}

std::size_t sq_mapped_packet_header_t::packet_size() const
{
    return length + sizeof(sq_mapped_packet_header_t);
}

bool sq_mapped_packet_header_t::is_valid() const
{
    return signature == sq_fragment_signature
            && padding == sq_fragment_header_padding;
}

bool sq_mapped_packet_header_t::is_fragment() const
{
    return packet_type == static_cast<std::uint8_t>(sq_packet_type_t::fragment);
}

bool sq_mapped_packet_header_t::is_request() const
{
    return packet_type == static_cast<std::uint8_t>(sq_packet_type_t::request_nack);
}

sq_packet_type_t sq_mapped_packet_header_t::type() const
{
    return packet_type <= static_cast<std::uint8_t>(sq_packet_type_t::request_nack)
            ? static_cast<sq_packet_type_t>(packet_type)
            : sq_packet_type_t::undefined;
}

void sq_mapped_packet_header_t::tune()
{
    signature = sq_fragment_signature;
    padding = sq_fragment_header_padding;
}

}
