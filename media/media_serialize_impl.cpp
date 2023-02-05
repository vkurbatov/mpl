#include "core/property_writer.h"

#include "audio_format_impl.h"
#include "video_format_impl.h"

namespace mpl
{

using namespace media;

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
