#ifndef MPL_MEDIA_DEVICE_INFO_H
#define MPL_MEDIA_DEVICE_INFO_H

#include "media_types.h"
#include "stream_types.h"
#include <string>
#include <vector>

namespace mpl::media
{

struct device_info_t
{
    using array_t = std::vector<device_info_t>;

    static std::vector<std::string> device_class_list(media_type_t media_type
                                                     , stream_direction_t direction);
    static array_t device_list(media_type_t media_type = media_type_t::undefined
                               , stream_direction_t direction = stream_direction_t::undefined
                               , const std::string_view& device_class = {});

    media_type_t        media_type;
    std::string         name;
    std::string         description;
    std::string         device_class;
    stream_direction_t  direction;

    device_info_t(media_type_t media_type = media_type_t::undefined
                  , const std::string_view& name = {}
                  , const std::string_view& description = {}
                  , const std::string_view& device_class = {}
                  , stream_direction_t direction = stream_direction_t::undefined);

    bool operator == (const device_info_t& other) const;
    bool operator != (const device_info_t& other) const;

    bool is_valid() const;
};

}

#endif // MPL_MEDIA_DEVICE_INFO_H
