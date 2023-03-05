#include "core/property_writer.h"

#include "media_utils.h"

#include "audio_format_impl.h"
#include "video_format_impl.h"

namespace mpl
{

using namespace media;

// audio_format_impl
template<>
bool property_helper::serialize(const audio_format_impl& value, i_property& property)
{
    return value.get_params(property);
    /*
    property_writer writer(property);

    if (writer.set("media_type", value.media_type())
            && writer.set("format", value.format_id())
            && writer.set("sample_rate", value.sample_rate())
            && writer.set("channels", value.channels()))
    {
        utils::convert_format_options(value.options(), property);
        return true;
    }

    return false;*/
}

template<>
bool property_helper::deserialize(audio_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
    /*
    property_reader reader(property);

    return reader.get("media_type", value.media_type())
            | reader.get("format", value.format_id())
            | reader.get("sample_rate", value.sample_rate())
            | reader.get("channels", value.channels())
            | utils::convert_format_options(property, value.options());*/
}

// video_format_impl
template<>
bool property_helper::serialize(const video_format_impl& value
                                , i_property& property)
{
    return value.get_params(property);
    /*
    property_writer writer(property);

    if (writer.set("media_type", value.media_type())
            && writer.set("format", value.format_id())
            && writer.set("width", value.width())
            && writer.set("height", value.height())
            && writer.set("frame_rate", value.frame_rate()))
    {
        utils::convert_format_options(value.options(), property);
        return true;
    }

    return false;*/
}

template<>
bool property_helper::deserialize(video_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
    /*
    property_reader reader(property);

    return reader.get("media_type", value.media_type())
            | reader.get("format", value.format_id())
            | reader.get("width", value.width())
            | reader.get("height", value.height())
            | reader.get("frame_rate", value.frame_rate())
            | utils::convert_format_options(property, value.options());*/
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
