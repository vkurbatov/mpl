#include "core/property_writer.h"

#include "media_utils.h"

#include "audio_format_impl.h"
#include "video_format_impl.h"
#include "video_frame_types.h"

namespace mpl
{

using namespace media;

// audio_format_impl
template<>
bool property_helper::serialize(const audio_format_impl& value, i_property& property)
{
    return value.get_params(property);
}

template<>
bool property_helper::deserialize(audio_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
}


// video_format_impl
template<>
bool property_helper::serialize(const video_format_impl& value
                                , i_property& property)
{
    return value.get_params(property);

}

template<>
bool property_helper::deserialize(video_format_impl& value
                                  , const i_property& property)
{
    return value.set_params(property);
}

// frame_point_t
template<>
bool property_helper::serialize(const frame_point_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("x", value.x)
            && writer.set("y", value.y);

}

template<>
bool property_helper::deserialize(frame_point_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("x", value.x)
            | reader.get("y", value.y);
}

// frame_size_t
template<>
bool property_helper::serialize(const frame_size_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("width", value.width)
            && writer.set("height", value.height);

}

template<>
bool property_helper::deserialize(frame_size_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("width", value.width)
            | reader.get("height", value.height);
}

// frame_rect_t
template<>
bool property_helper::serialize(const frame_rect_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("offset", value.offset)
            && writer.set("size", value.size);

}

template<>
bool property_helper::deserialize(frame_rect_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("offset", value.offset)
            | reader.get("size", value.size);
}

// relative_frame_point_t
template<>
bool property_helper::serialize(const relative_frame_point_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("x", value.x)
            && writer.set("y", value.y);

}

template<>
bool property_helper::deserialize(relative_frame_point_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("x", value.x)
            | reader.get("y", value.y);
}

// relative_frame_size_t
template<>
bool property_helper::serialize(const relative_frame_size_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("width", value.width)
            && writer.set("height", value.height);

}

template<>
bool property_helper::deserialize(relative_frame_size_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("width", value.width)
            | reader.get("height", value.height);
}

// relative_frame_rect_t
template<>
bool property_helper::serialize(const relative_frame_rect_t& value
                                , i_property& property)
{
    property_writer writer(property);
    return writer.set("offset", value.offset)
            && writer.set("size", value.size);

}

template<>
bool property_helper::deserialize(relative_frame_rect_t& value
                                  , const i_property& property)
{
    property_reader reader(property);
    return reader.get("offset", value.offset)
            | reader.get("size", value.size);
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
