#ifndef OCV_FRAME_INFO_T_H
#define OCV_FRAME_INFO_T_H

#include "ocv_types.h"

namespace ocv
{

struct frame_info_t
{
    frame_format_t  format;
    frame_size_t    size;

    frame_info_t(const frame_format_t& format = frame_format_t::undefined
                 , const frame_size_t& size = {});

    bool operator == (const frame_info_t& other) const;
    bool operator != (const frame_info_t& other) const;

    std::size_t frame_size() const;
    const std::string& format_name() const;
    bool is_valid() const;
};

}

#endif // OCV_FRAME_INFO_T_H
