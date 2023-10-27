#ifndef PT_CODEC_H264_MAPPED_HEADERS_H
#define PT_CODEC_H264_MAPPED_HEADERS_H

#include "h264_types.h"
#include <cstdint>

namespace pt::codec
{

#pragma pack(push,1)

union fragment_mapped_header_t
{
    std::uint32_t   annex_b_header_32;
    std::uint32_t   annex_b_header_24:24;
    std::uint32_t   avcc_header_32;
    std::uint32_t   avcc_header_24:24;
    std::uint32_t   avcc_header_16:16;
    std::uint32_t   avcc_header_8:8;
};

struct avc_mapped_header_t
{
    std::uint8_t    conf_version;
    std::uint8_t    avc_profile_indication;
    std::uint8_t    profile_compatibility;
    std::uint8_t    avc_level_indication;
    std::uint8_t    length_size_minus_one:2;
    std::uint8_t    reserved_1:6;
    std::uint8_t    number_of_sps:5;
    std::uint8_t    reserved_2:3;

    h264_fragmentation_type_t fragmentation_type() const;
    bool is_valid() const;
};

struct nal_mapped_header_t
{
    std::uint8_t    nal:5;
    std::uint8_t    nri:2;
    std::uint8_t    f:1;

    h264_nalu_type_t nalu_type() const;
    const void* payload() const;
    void* payload();
};

struct stap_a_header_t
{
    std::uint16_t   length;
};

struct fu_a_header_t
{
    std::uint8_t    nal:5;
    std::uint8_t    f:1;
    std::uint8_t    end:1;
    std::uint8_t    start:1;

    h264_nalu_type_t nalu_type() const;
};

#pragma pack(pop)

}

#endif // PT_CODEC_H264_MAPPED_HEADERS_H
