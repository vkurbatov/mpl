#include "convert_utils.h"
#include "enum_utils.h"
#include "property_helper.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"


namespace mpl
{

#define declare_enum_serializer(enum_type)\
    template bool property_helper::serialize(const enum_type& enum_value, i_property& property);\
    template<> i_property::u_ptr_t property_helper::serialize(const enum_type& enum_value) { return property_helper::serialize(utils::enum_to_string(enum_value)); }\
    template bool property_helper::deserialize(enum_type& enum_value, const i_property& property);\

declare_enum_serializer(device_type_t)
declare_enum_serializer(media_type_t)
declare_enum_serializer(audio_format_id_t)
declare_enum_serializer(video_format_id_t)

template<typename E>
bool property_helper::serialize(const E& enum_value, i_property& property)
{
    return property_helper::serialize(utils::enum_to_string(enum_value)
                                      , property);
}

template<typename E>
bool property_helper::deserialize(E& enum_value, const i_property& property)
{
    std::string string_value;
    return property_helper::deserialize(string_value
                                        , property)
            && utils::convert(string_value, enum_value);
}

}
