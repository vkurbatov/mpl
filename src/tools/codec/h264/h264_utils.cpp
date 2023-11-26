#include "h264_utils.h"
#include "h264_mapped_headers.h"
#include "tools/utils/endian_base.h"

#include <cstring>

namespace pt::codec
{

namespace detail
{

inline bool has_annex_b(std::uint32_t start_code)
{
    return start_code == annex_b_start_code
            || (start_code << 8) == annex_b_start_code;
}

h264_fragment_t::array_t split_annex_b(const void *frame_data
                                      , std::size_t frame_size)
{
    h264_fragment_t::array_t fragment_list;
    const auto* ptr = static_cast<const std::uint8_t*>(frame_data);

    if (frame_size > annex_b_min_frame_size
            && ptr[0] == 0
            && ptr[1] == 0
            && (ptr[2] == 1
                || (ptr[2] == 0
                    && ptr[3] == 1)
                )
            )
    {
        for (std::size_t i = 0; i < frame_size - annex_b_min_frame_size;)
        {
            if (ptr[i + 2] > 1)
            {
                i += annex_b_min_header_size;
            }
            else if (ptr[i + 2] == 1
                     && ptr[i + 1] == 0
                     && ptr[i + 0] == 0)
            {
                h264_fragment_t fragment(i
                                         , i + annex_b_min_header_size);
                if (i > 0 && ptr[i - 1] == 0)
                {
                    fragment.offset--;
                }

                if (!fragment_list.empty())
                {
                    fragment_list.back().payload_length = fragment.offset - fragment_list.back().payload_offset;
                }

                fragment_list.push_back(fragment);

                i += annex_b_min_header_size;
            }
            else
            {
                i++;
            }
        }

        if (!fragment_list.empty())
        {
            fragment_list.back().payload_length = frame_size
                    - fragment_list.back().payload_offset;
        }
    }

    return fragment_list;
}

h264_fragment_t::array_t split_avcc(std::size_t header_size
                                    , const void *frame_data
                                    , std::size_t frame_size)
{
    h264_fragment_t::array_t fragment_list;
    const auto* start = static_cast<const std::uint8_t*>(frame_data);
    const auto* ptr = start;

    while (frame_size >= header_size)
    {
        std::uint32_t fragment_size = 0;
        std::memcpy(&fragment_size, ptr, header_size);
        pt::utils::convert_order(utils::octet_order_t::big_endian, &fragment_size, header_size);

        frame_size -= header_size;

        if (fragment_size > frame_size)
        {
            break;
        }

        fragment_list.emplace_back(ptr - start
                                   , ptr - start + header_size
                                   , fragment_size);
        ptr += fragment_size + header_size;
        frame_size -= fragment_size;
    }

    if (frame_size != 0)
    {
        fragment_list.clear();
    }

    return fragment_list;
}

}

h264_fragmentation_type_t get_fragmentation_type(const void *data, std::size_t size)
{
    if (size > 4)
    {
        if (detail::has_annex_b(*static_cast<const std::uint32_t*>(data)))
        {
            return h264_fragmentation_type_t::annex_b;
        }
        else if (size >= sizeof(avc_mapped_header_t))
        {
            const auto& header = *static_cast<const avc_mapped_header_t*>(data);
            if (header.is_valid())
            {
                return header.fragmentation_type();
            }
        }
    }

    return h264_fragmentation_type_t::undefined;

}

h264_fragment_t::array_t split_fragments(h264_fragmentation_type_t type
                                         , const void *data
                                         , std::size_t size)
{
    switch(type)
    {
        case h264_fragmentation_type_t::annex_b:
            return detail::split_annex_b(data, size);
        break;
        case h264_fragmentation_type_t::avcc_8:
        case h264_fragmentation_type_t::avcc_16:
        case h264_fragmentation_type_t::avcc_24:
        case h264_fragmentation_type_t::avcc_32:
            return detail::split_avcc(static_cast<std::size_t>(type)
                                      , data
                                      , size);
        break;
        default:;
    }

    return {};
}

}
