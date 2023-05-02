#include "mapped_seq_header.h"

namespace mpl::seq
{

mapped_packet_header_t::mapped_packet_header_t(packet_type_t packet_type
                                               , uint16_t id
                                               , uint32_t length)
    : signature(fragment_signature)
    , padding(fragment_header_padding)
    , id(id)
    , length(length)
{
    set_packet_type(packet_type);
}

void mapped_packet_header_t::set_packet_type(packet_type_t packet_type)
{
    if (packet_type != packet_type_t::undefined)
    {
        this->packet_type = static_cast<std::uint8_t>(packet_type);
    }
}

std::size_t mapped_packet_header_t::packet_size() const
{
    return length + sizeof(mapped_packet_header_t);
}

bool mapped_packet_header_t::is_valid() const
{
    return signature == fragment_signature
            && padding == fragment_header_padding;
}

bool mapped_packet_header_t::is_fragment() const
{
    return packet_type == static_cast<std::uint8_t>(packet_type_t::fragment);
}

bool mapped_packet_header_t::is_request() const
{
    return packet_type == static_cast<std::uint8_t>(packet_type_t::request_nack);
}

packet_type_t mapped_packet_header_t::type() const
{
    return packet_type <= static_cast<std::uint8_t>(packet_type_t::request_nack)
            ? static_cast<packet_type_t>(packet_type)
            : packet_type_t::undefined;
}

void mapped_packet_header_t::tune()
{
    signature = fragment_signature;
    padding = fragment_header_padding;
}

}
