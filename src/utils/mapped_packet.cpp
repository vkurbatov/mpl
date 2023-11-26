#include "mapped_packet.h"
#include <cstring>

namespace mpl
{

constexpr std::uint8_t max_inbound_size = 0x1c;

std::size_t mapped_packet_t::calculate_header_size(size_t payload_size)
{
    std::size_t result = sizeof(header_t);
    if (payload_size >= max_inbound_size)
    {
        while(payload_size > 0)
        {
            result++;
            payload_size >>= 8;
        }
    }

    return result;
}

std::size_t mapped_packet_t::calculate_packet_size(size_t payload_size)
{
    return calculate_header_size(payload_size) + payload_size;
}

mapped_packet_t::packet_data_t mapped_packet_t::build_packet(field_type_t type
                                                             , const void *payload_data
                                                             , std::size_t payload_size)
{
    packet_data_t packet_data(calculate_packet_size(payload_size));
    mapped_packet_t& packet = *reinterpret_cast<mapped_packet_t*>(packet_data.data());
    packet.write_packet(type
                        , payload_data
                        , payload_size);

    return packet_data;
}

std::size_t mapped_packet_t::header_t::header_size() const
{
    return sizeof(header_t) + external_size();
}

std::size_t mapped_packet_t::header_t::external_size() const
{
    return length >= max_inbound_size
            ? (length & 0x03) + 1
            : 0;
}

const void *mapped_packet_t::payload_data() const
{
    return reinterpret_cast<const std::uint8_t*>(this) + header.header_size();
}

void* mapped_packet_t::payload_data()
{
    return reinterpret_cast<std::uint8_t*>(this) + header.header_size();
}

std::size_t mapped_packet_t::payload_size() const
{
    std::size_t payload_size = 0;

    auto external_size = header.external_size();

    if (external_size == 0)
    {
        payload_size = header.length;
    }
    else
    {
        const auto data = reinterpret_cast<const std::uint8_t*>(this) + sizeof(header_t);
        std::memcpy(&payload_size
                    , data
                    , external_size);
    }

    return payload_size;
}

std::size_t mapped_packet_t::packet_size() const
{
    return header.header_size() + payload_size();
}

bool mapped_packet_t::is_valid(std::size_t packet_size) const
{
    return packet_size > 0
            && packet_size >= header.header_size()
            && packet_size == this->packet_size();
}

std::size_t mapped_packet_t::read_packet(field_type_t type
                                         , void* payload_data
                                         , size_t payload_size) const
{
    std::size_t result = 0;

    if (type == header.field_type)
    {
        auto pack_size = std::min(payload_size
                                  , this->payload_size());

        if (payload_data != nullptr)
        {
            if (pack_size > 0)
            {
                std::memcpy(payload_data
                            , this->payload_data()
                            , pack_size);
            }

            if (pack_size < payload_size)
            {
                std::memset(static_cast<std::uint8_t*>(payload_data) + pack_size
                            , 0
                            , payload_size - pack_size);
            }
        }

        result = payload_size;
    }

    return result;
}

std::size_t mapped_packet_t::write_packet(field_type_t type
                                          , const void *payload_data
                                          , std::size_t payload_size)
{
    auto header_size = calculate_header_size(payload_size);
    auto extension_size = header_size - sizeof(header_t);
    header.field_type = type;

    auto user_data = reinterpret_cast<std::uint8_t*>(this) + sizeof(header_t);

    if (extension_size == 0)
    {
        header.length = payload_size;
    }
    else
    {
        header.length = max_inbound_size + (extension_size - 1);
        std::memcpy(user_data
                    , &payload_size
                    , extension_size);

        user_data += extension_size;
    }

    if (payload_data != nullptr)
    {
        std::memcpy(user_data
                    , payload_data
                    , payload_size);
    }

    return header_size + payload_size;
}

}
