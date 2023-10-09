#ifndef MPL_ENUM_SERIALIZE_DEFS_H
#define MPL_ENUM_SERIALIZE_DEFS_H

#include "convert_utils.h"
#include "property_utils.h"
#include "enum_utils.h"
#include "property_helper.h"

namespace mpl
{

namespace detail
{

template<typename E>
bool serialize(const E& enum_value, i_property& property)
{
    return utils::property::serialize(utils::enum_to_string(enum_value)
                                      , property);
}

template<typename E>
i_property::u_ptr_t serialize(const E& enum_value)
{
    return utils::property::serialize(utils::enum_to_string(enum_value));
}


template<typename E>
bool deserialize(E& enum_value, const i_property& property)
{
    std::string string_value;
    return utils::property::deserialize(string_value
                                        , property)
            && utils::convert(string_value, enum_value);
}

}

#define declare_enum_serializer(enum_type)\
    template<> bool utils::property::serialize(const enum_type& enum_value, i_property& property) { return mpl::detail::serialize(enum_value, property); };\
    template<> i_property::u_ptr_t utils::property::serialize(const enum_type& enum_value) { return mpl::detail::serialize(enum_value); }\
    template<> bool utils::property::deserialize(enum_type& enum_value, const i_property& property) { return mpl::detail::deserialize(enum_value, property); };;

}

#endif // MPL_ENUM_SERIALIZE_DEFS_H
