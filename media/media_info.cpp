#include "media_info.h"

namespace mpl::media
{

media_info_t::media_info_t()
    : media_type(media_type_t::undefined)
{

}

media_info_t::media_info_t(const audio_info_t &audio_info)
    : media_type(media_type_t::audio)
    , audio_info(audio_info)
{

}

media_info_t::media_info_t(const video_info_t &video_info)
    : media_type(media_type_t::video)
    , video_info(video_info)
{

}

media_info_t::~media_info_t()
{
    switch(media_type)
    {
        case media_type_t::audio:
            audio_info.~audio_info_t();
        break;
        case media_type_t::video:
            video_info.~video_info_t();
        break;
        default:;
    }
}

bool media_info_t::operator ==(const media_info_t &other) const
{
    if (media_type == other.media_type)
    {
        switch(media_type)
        {
            case media_type_t::audio:
                return audio_info == other.audio_info;
            break;
            case media_type_t::video:
                return video_info == other.video_info;
            break;
            default:
                return true;
        }
    }

    return false;
}

bool media_info_t::operator !=(const media_info_t &other) const
{
    return ! operator == (other);
}

bool media_info_t::is_valid() const
{
    switch(media_type)
    {
        case media_type_t::audio:
            return audio_info.is_valid();
        break;
        case media_type_t::video:
            return video_info.is_valid();
        break;
    }

    return true;
}

bool media_info_t::is_compatibe(const media_info_t& other) const
{
    if (media_type == other.media_type)
    {
        switch(media_type)
        {
            case media_type_t::audio:
                return audio_info.is_compatible(other.audio_info);
            break;
            case media_type_t::video:
                return video_info.is_compatible(other.video_info);
            break;
            default:
                return true;
        }
    }

    return false;
}

std::string media_info_t::to_string() const
{
    switch(media_type)
    {
        case media_type_t::audio:
            return audio_info.to_string();
        break;
        case media_type_t::video:
            return video_info.to_string();
        break;
        default:;
    }

    return {};
}


}
