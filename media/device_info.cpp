#include "device_info.h"
#include "tools/ffmpeg/libav_base.h"

namespace mpl::media
{

std::vector<std::string> device_info_t::device_class_list(media_type_t media_type
                                                         , stream_direction_t direction)
{
    if (pt::ffmpeg::is_registered()
            && direction != stream_direction_t::undefined)
    {
        switch(media_type)
        {
            case media_type_t::audio:
                return pt::ffmpeg::device_info_t::device_class_list(pt::ffmpeg::media_type_t::audio
                                                                    , direction == stream_direction_t::input);
            break;
            case media_type_t::video:
                return pt::ffmpeg::device_info_t::device_class_list(pt::ffmpeg::media_type_t::video
                                                                    , direction == stream_direction_t::input);
            break;
            default:;
        }
    }

    return {};

}

device_info_t::array_t device_info_t::device_list(media_type_t media_type
                                                  , stream_direction_t direction
                                                  , const std::string_view &device_class)
{
    device_info_t::array_t list;

    if (media_type == media_type_t::undefined)
    {
        for (auto mt : { media_type_t::audio, media_type_t::video })
        {
            auto dl = device_list(mt
                                  , direction
                                  , device_class);
            list.insert(list.end()
                        , dl.begin()
                        , dl.end());
        }
    }
    else if (direction == stream_direction_t::undefined)
    {
        for (auto d : { stream_direction_t::input, stream_direction_t::output })
        {
            auto dl = device_list(media_type
                                  , d
                                  , device_class);
            list.insert(list.end()
                       , dl.begin()
                       , dl.end());
        }
    }
    else
    {
        switch(media_type)
        {
            case media_type_t::audio:
            case media_type_t::video:
            {
                for (const auto& info : pt::ffmpeg::device_info_t::device_list(media_type == media_type_t::audio
                                                                               ? pt::ffmpeg::media_type_t::audio
                                                                               : pt::ffmpeg::media_type_t::video
                                                                               , direction == stream_direction_t::input
                                                                               , device_class))
                {
                    list.emplace_back(media_type
                                      , info.name
                                      , info.description
                                      , info.device_class
                                      , direction);
                }
            }
            break;
            default:;
        }

    }

    return list;
}

device_info_t::device_info_t(media_type_t media_type
                             , const std::string_view &name
                             , const std::string_view &description
                             , const std::string_view &device_class
                             , stream_direction_t direction)
    : media_type(media_type)
    , name(name)
    , description(description)
    , device_class(device_class)
    , direction(direction)
{

}

bool device_info_t::operator ==(const device_info_t &other) const
{
    return media_type == other.media_type
            && name == other.name
            && description == other.description
            && device_class == other.device_class
            && direction == other.direction;
}

bool device_info_t::operator !=(const device_info_t &other) const
{
    return !operator == (other);
}

bool device_info_t::is_valid() const
{
    return media_type != media_type_t::undefined
            && !name.empty()
            && direction != stream_direction_t::undefined;
}



}
