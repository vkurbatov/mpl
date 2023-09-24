#include "json_utils.h"
#include "property_tree_impl.h"
#include "property_value_impl.h"
#include "thrid_party/picojson/picojson.hpp"
#include "convert_utils.h"
#include "command_types.h"

namespace mpl::core::utils
{

namespace detail
{

i_property::u_ptr_t fetch_property(const picojson::value& value)
{
    switch(value.get_type())
    {
        case picojson::object_type:
        {
            const auto& object = value.get<picojson::object>();
            property_tree::property_map_t propertyes;
            for (const auto& v : object)
            {
                propertyes[v.first] = fetch_property(v.second);
            }
            return property_tree::create(propertyes);
        }
        break;
        case picojson::number_type:
            return property_value<double>::create(value.get<double>());
        break;
        case picojson::array_type:
        {
            i_property::s_array_t array;
            for (const auto& v : value.get<picojson::array>())
            {
                if (auto value = fetch_property(v))
                {
                    array.emplace_back(std::move(value));
                }
            }
            return property_value<i_property::s_array_t>::create(std::move(array));
        }
        break;
        case picojson::string_type:
            return property_value<std::string>::create(value.get<std::string>());
        break;
        case picojson::boolean_type:
            return property_value<bool>::create(value.get<bool>());
        break;
    }

    return nullptr;
}

i_property::u_ptr_t deserialize(const std::string &json_string)
{
    picojson::value     json_value;
    auto err = picojson::parse(json_value, json_string);
    if (err.empty())
    {
        return fetch_property(json_value);
    }
    else
    {
        // error
    }

    return nullptr;
}

picojson::value add_property(const i_property& property)
{
    switch(property.property_type())
    {
        case property_type_t::object:
        {
            picojson::object object;
            const auto& tree = static_cast<const i_property_tree&>(property);
            for (const auto& key : tree.property_list())
            {
                if (auto p = tree.get(key))
                {
                    picojson::value value = add_property(*p.get());
                    if (!value.is<picojson::null>())
                    {
                        object.emplace(key, std::move(value));
                    }
                }
            }
            return picojson::value(object);
        }
        break;
        case property_type_t::array:
        {
            picojson::array array;
            const auto& p_array = static_cast<const i_property_value<i_property::s_array_t>&>(property);
            for (const auto& v : p_array.get_value())
            {
                if (v != nullptr)
                {
                    picojson::value value = add_property(*v.get());
                    if (!value.is<picojson::null>())
                    {
                        array.emplace_back(std::move(value));
                    }
                }
            }

            return picojson::value(array);
        }
        break;
        case property_type_t::i8:
        case property_type_t::i16:
        case property_type_t::i32:
        case property_type_t::i64:
        case property_type_t::u8:
        case property_type_t::u16:
        case property_type_t::u32:
        case property_type_t::u64:
        case property_type_t::r32:
        case property_type_t::r64:
        case property_type_t::r96:
        {
            double value = 0.0;
            if (utils::convert(property, value))
            {
                return picojson::value(value);
            }
        }
        break;
        case property_type_t::boolean:
        {
            const auto& boolean_value = static_cast<const i_property_value<bool>&>(property);
            return picojson::value(boolean_value.get_value());
        }
        break;
        case property_type_t::string:
        {
            const auto& string_value = static_cast<const i_property_value<std::string>&>(property);
            return picojson::value(string_value.get_value());
        }
        break;
        default:;
    }

    return {};
}


}

i_property::u_ptr_t from_json(const std::string &json_string)
{
    return detail::deserialize(json_string);
}

std::string to_json(i_property &property, bool prettify)
{
    auto value = detail::add_property(property);
    return value.serialize(prettify);
}

}
