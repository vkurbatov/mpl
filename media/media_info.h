#ifndef MPL_MEDIA_INFO_H
#define MPL_MEDIA_INFO_H

#include "media_types.h"
#include "audio_info.h"
#include "video_info.h"
#include <variant>

namespace mpl::media
{

struct media_info_t
{
    media_type_t    media_type;
    union
    {
        audio_info_t    audio_info;
        video_info_t    video_info;
    };

    media_info_t();
    media_info_t(const audio_info_t& audio_info);
    media_info_t(const video_info_t& video_info);
    ~media_info_t();

    bool operator == (const media_info_t& other) const;
    bool operator != (const media_info_t& other) const;

    bool is_valid() const;
    bool is_compatibe(const media_info_t& other) const;

    std::string to_string() const;
};

}

#endif // MPL_MEDIA_INFO_H
