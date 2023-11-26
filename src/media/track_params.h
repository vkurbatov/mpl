#ifndef MPL_MEDIA_TRACK_PARAMS_H
#define MPL_MEDIA_TRACK_PARAMS_H

#include <string>

namespace mpl::media
{

struct track_params_t
{
    std::string         name;
    bool                enabled;

    track_params_t(const std::string_view& name = {}
            , bool enabled = false);

    bool operator == (const track_params_t& other) const;
    bool operator != (const track_params_t& other) const;
};

}

#endif // MPL_MEDIA_TRACK_PARAMS_H
