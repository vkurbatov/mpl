#include "utils/property_writer.h"
#include "utils/property_utils.h"

#include "media_utils.h"

#include "audio_format_impl.h"
#include "video_format_impl.h"
#include "video_frame_types.h"

#include "track_info.h"

#include "image_frame.h"
#include "audio_sample.h"

#include "audio_info.h"
#include "video_info.h"

#include "track_params.h"
#include "mcu/compose_audio_track_params.h"
#include "mcu/compose_video_track_params.h"
#include "mcu/compose_stream_params.h"

namespace mpl
{

using namespace media;

// audio_info_t
template<>
bool utils::property::serialize(const audio_info_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("format", value.format_id)
            && writer.set("sample_rate", value.sample_rate)
            && writer.set("channels", value.channels);
}

template<>
bool utils::property::deserialize(audio_info_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("format", value.format_id)
            | reader.get("sample_rate", value.sample_rate)
            | reader.get("channels", value.channels);
}

// i_audio_format
template<>
bool utils::property::serialize(const i_audio_format& value, i_property& property)
{
    property_writer writer(property);

    if (writer.set("media_type", media_type_t::audio)
            && writer.set("format", value.format_id())
            && writer.set("sample_rate", value.sample_rate())
            && writer.set("channels", value.channels()))
    {
        utils::convert_format_options(value.options(), property);
        return true;
    }

    return false;
}

// audio_format_impl
template<>
bool utils::property::serialize(const audio_format_impl& value, i_property& property)
{
    return value.get_params(property);
}

template<>
bool utils::property::deserialize(audio_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
}

// video_info_t
template<>
bool utils::property::serialize(const video_info_t& value, i_property& property)
{
    property_writer writer(property);
    return writer.set("format", value.format_id)
            && writer.set("width", value.size.width)
            && writer.set("height", value.size.height)
            && writer.set("frame_rate", value.frame_rate);
}

template<>
bool utils::property::deserialize(video_info_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("format", value.format_id)
            | reader.get("width", value.size.width)
            | reader.get("height", value.size.height)
            | reader.get("frame_rate", value.frame_rate);
}

// i_video_format
template<>
bool utils::property::serialize(const i_video_format& value, i_property& property)
{
    property_writer writer(property);

    if (writer.set("media_type", media_type_t::video)
            && writer.set("format", value.format_id())
            && writer.set("width", value.width())
            && writer.set("height", value.height())
            && writer.set("frame_rate", value.frame_rate()))
    {
        utils::convert_format_options(value.options(), property);
        return true;
    }

    return false;
}

// video_format_impl
template<>
bool utils::property::serialize(const video_format_impl& value
                                , i_property& property)
{
    return value.get_params(property);

}

template<>
bool utils::property::deserialize(video_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
}

// i_media_format
template<>
bool utils::property::serialize(const i_media_format& value
                                , i_property& property)
{
    switch(value.media_type())
    {
        case media_type_t::audio:
            return serialize(static_cast<const i_audio_format&>(value)
                             , property);
        break;
        case media_type_t::video:
            return serialize(static_cast<const i_video_format&>(value)
                             , property);
        break;
        default:;
    }

    return false;
}


// frame_point_t
template<>
bool utils::property::serialize(const frame_point_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("x", value.x)
            && writer.set("y", value.y);

}

template<>
bool utils::property::deserialize(frame_point_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("x", value.x)
            | reader.get("y", value.y);
}

// frame_size_t
template<>
bool utils::property::serialize(const frame_size_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("width", value.width)
            && writer.set("height", value.height);

}

template<>
bool utils::property::deserialize(frame_size_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("width", value.width)
            | reader.get("height", value.height);
}

// frame_rect_t
template<>
bool utils::property::serialize(const frame_rect_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("offset", value.offset)
            && writer.set("size", value.size);

}

template<>
bool utils::property::deserialize(frame_rect_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("offset", value.offset)
            | reader.get("size", value.size);
}

// relative_frame_point_t
template<>
bool utils::property::serialize(const relative_frame_point_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("x", value.x)
            && writer.set("y", value.y);

}

template<>
bool utils::property::deserialize(relative_frame_point_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("x", value.x)
            | reader.get("y", value.y);
}

// relative_frame_size_t
template<>
bool utils::property::serialize(const relative_frame_size_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("width", value.width)
            && writer.set("height", value.height);

}

template<>
bool utils::property::deserialize(relative_frame_size_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("width", value.width)
            | reader.get("height", value.height);
}

// relative_frame_rect_t
template<>
bool utils::property::serialize(const relative_frame_rect_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("offset", value.offset)
            && writer.set("size", value.size);

}

template<>
bool utils::property::deserialize(relative_frame_rect_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("offset", value.offset)
            | reader.get("size", value.size);
}

// track_info_t
template<>
bool utils::property::serialize(const track_info_t& value, i_property& property)
{
    property_writer writer(property);
    return writer.set("stream_id", value.stream_id)
            && writer.set("track_id", value.track_id)
            && writer.set("layer_id", value.layer_id);
}

template<>
bool utils::property::deserialize(track_info_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("stream_id", value.stream_id)
            | reader.get("track_id", value.track_id)
            | reader.get("layer_id", value.layer_id);
}

// track_params_t
template<>
bool utils::property::serialize(const track_params_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("name", value.name)
            && writer.set("enabled", value.enabled);
}

template<>
bool utils::property::deserialize(track_params_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("name", value.name)
            | reader.get("enabled", value.enabled);
}

// draw_options_t
template<>
bool utils::property::serialize(const draw_options_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("rect", value.target_rect)
            && writer.set("opacity", value.opacity)
            && writer.set("border", value.border)
            && writer.set("margin", value.margin)
            && writer.set("label", value.label)
            && writer.set("elliptic", value.elliptic);
}

template<>
bool utils::property::deserialize(draw_options_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("rect", value.target_rect)
            | reader.get("opacity", value.opacity)
            | reader.get("border", value.border)
            | reader.get("margin", value.margin)
            | reader.get("label", value.label)
            | reader.get("elliptic", value.elliptic);
}


// compose_audio_track_params_t
template<>
bool utils::property::serialize(const compose_audio_track_params_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set<track_params_t>({}, value)
            && writer.set("volume", value.volume);
}

template<>
bool utils::property::deserialize(compose_audio_track_params_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get<track_params_t>({}, value)
            | reader.get("volume", value.volume);
}

// compose_video_track_params_t
template<>
bool utils::property::serialize(const compose_video_track_params_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set<track_params_t>({}, value)
            && writer.set("draw_options", value.draw_options)
            && writer.set("animation", value.animation)
            && writer.set("user_img", value.user_image_path)
            && writer.set("timeout", value.timeout);
}

template<>
bool utils::property::deserialize(compose_video_track_params_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get<track_params_t>({}, value)
            | reader.get("draw_options", value.draw_options)
            | reader.get("animation", value.animation)
            | reader.get("user_img", value.user_image_path)
            | reader.get("timeout", value.timeout);
}

// compose_stream_params_t
template<>
bool utils::property::serialize(const compose_stream_params_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("order", value.order)
            && writer.set("name", value.name)
            && writer.set("audio_track", value.audio_track)
            && writer.set("video_track", value.video_track);
}

template<>
bool utils::property::deserialize(compose_stream_params_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("order", value.order)
            | reader.get("name", value.name)
            | reader.get("audio_track", value.audio_track)
            | reader.get("video_track", value.video_track);
}

/*
template<typename E>
bool property_helper::serialize(const E& enum_value, i_property& property)
{
    return property_helper::serialize(core::utils::enum_to_string(enum_value)
                                      , property);
}

template<typename E>
i_property::u_ptr_t property_helper::serialize(const E& enum_value)
{
    return property_helper::serialize(core::utils::enum_to_string(enum_value));
}


template<typename E>
bool property_helper::deserialize(E& enum_value, const i_property& property)
{
    std::string string_value;
    return property_helper::deserialize(string_value
                                        , property)
            && core::utils::convert(string_value, enum_value);
}
*/

}
