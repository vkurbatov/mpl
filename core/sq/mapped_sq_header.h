#ifndef MPL_MAPPED_SEQ_HEADER_H
#define MPL_MAPPED_SEQ_HEADER_H

#include "sq_types.h"

namespace mpl::sq
{

#pragma pack(push, 1)

constexpr std::uint8_t fragment_signature = 0x7a;
constexpr std::uint8_t fragment_header_padding = 0x07;


struct mapped_packet_header_t
{
    std::uint8_t        signature;
    std::uint8_t        session_id;
    std::uint8_t        packet_type:3;
    std::uint8_t        head:1;
    std::uint8_t        tail:1;
    std::uint8_t        padding:3;
    std::uint16_t       id;
    std::uint32_t       length;


    mapped_packet_header_t(packet_type_t packet_type = packet_type_t::undefined
                          , std::uint16_t id = 0
                          , std::uint32_t length = 0);

    void set_packet_type(packet_type_t packet_type);
    std::size_t packet_size() const;

    bool is_valid() const;

    bool is_fragment() const;
    bool is_request() const;

    packet_type_t type() const;

    void tune();

};

struct mapped_nack_record_t
{
    std::uint16_t   id;
    std::uint8_t    ids[0];
};

#pragma pack(pop)

}

#endif // MAPPED_SEQ_HEADER_H
