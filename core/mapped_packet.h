#ifndef MPL_MAPPED_PACKET_H
#define MPL_MAPPED_PACKET_H

#include "packet_types.h"
#include "common_types.h"

namespace mpl
{

#pragma pack(push, 1)

struct mapped_packet_t
{
    using packet_data_t = std::vector<std::uint8_t>;

    static std::size_t calculate_header_size(std::size_t payload_size);
    static std::size_t calculate_packet_size(std::size_t payload_size);

    packet_data_t build_packet(field_type_t type
                               , const void* payload_data
                               , std::size_t payload_size);

    struct header_t
    {
        field_type_t    field_type:3;
        std::uint8_t    length:5;
        std::size_t     header_size() const;
        std::size_t     external_size() const;
    }                   header;


    static constexpr std::size_t min_header_size = sizeof(header_t);
    static constexpr std::size_t max_header_size = min_header_size + sizeof(std::size_t);

    const void* payload_data() const;
    void* payload_data();
    std::size_t payload_size() const;
    std::size_t packet_size() const;

    bool is_valid(std::size_t packet_size) const;

    std::size_t read_packet(field_type_t type
                            , void* payload_data
                            , std::size_t payload_size) const;

    std::size_t write_packet(field_type_t type
                             , const void* payload_data
                             , std::size_t payload_size);

};

#pragma pack(pop)

}

#endif // MPL_MAPPED_PACKET_H
