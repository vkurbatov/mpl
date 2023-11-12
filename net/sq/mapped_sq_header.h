#ifndef MPL_NET_MAPPED_SEQ_HEADER_H
#define MPL_NET_MAPPED_SEQ_HEADER_H

#include "sq_types.h"

namespace mpl::net
{

#pragma pack(push, 1)

constexpr std::uint8_t sq_fragment_signature = 0x7a;
constexpr std::uint8_t sq_fragment_header_padding = 0x07;


struct sq_mapped_packet_header_t
{
    std::uint8_t        signature;
    std::uint8_t        session_id;
    std::uint8_t        packet_type:3;
    std::uint8_t        head:1;
    std::uint8_t        tail:1;
    std::uint8_t        padding:3;
    std::uint16_t       id;
    std::uint32_t       length;


    sq_mapped_packet_header_t(sq_packet_type_t packet_type = sq_packet_type_t::undefined
                          , std::uint16_t id = 0
                          , std::uint32_t length = 0);

    void set_packet_type(sq_packet_type_t packet_type);
    std::size_t packet_size() const;

    bool is_valid() const;

    bool is_fragment() const;
    bool is_request() const;

    sq_packet_type_t type() const;

    void tune();

};

struct sq_mapped_nack_record_t
{
    std::uint16_t   id;
    std::uint8_t    ids[0];
};

#pragma pack(pop)

}

#endif // MPL_NET_MAPPED_SEQ_HEADER_H
