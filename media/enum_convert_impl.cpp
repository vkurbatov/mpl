#include "convert_utils.h"
#include "utils.h"
#include "enum_utils.h"

#include "device_types.h"

#include <unordered_map>
#include <vector>

namespace mpl::utils
{

namespace detail
{
    template<typename E>
    class enum_converter
    {

        using table_list_t = std::vector<std::pair<E, std::string>>;
        using right_map_t = std::unordered_map<E, std::string>;
        using reverse_map_t = std::unordered_map<std::string, E>;

        right_map_t         m_right_map;
        reverse_map_t       m_reverse_map;


    public:

        static right_map_t create_right_map(const table_list_t& table)
        {
            right_map_t result_map;

            for (const auto& t : table)
            {
                result_map.emplace(t.first
                                   , to_lower(t.second));
            }

            return result_map;
        }

        static reverse_map_t create_reverse_map(const table_list_t& table)
        {
            reverse_map_t result_map;

            for (const auto& t : table)
            {
                result_map.emplace(to_lower(t.second)
                                   , t.first);
            }

            return result_map;
        }

        enum_converter(const table_list_t& table)
            : m_right_map(create_right_map(table))
            , m_reverse_map(create_reverse_map(table))
        {

        }

        bool convert(const E& enum_value, std::string& string_value) const
        {
            if (auto it = m_right_map.find(enum_value); it != m_right_map.end())
            {
                string_value = it->second;
                return true;
            }

            return false;
        }

        bool convert(const std::string& string_value, E& enum_value) const
        {
            if (auto it = m_reverse_map.find(to_lower(string_value)); it != m_reverse_map.end())
            {
                enum_value = it->second;
                return true;
            }

            return false;
        }
    };

    template<typename E>
    std::vector<std::pair<E, std::string>> create_consistent_table(const std::vector<std::string>& names_table
                                                                   , std::int32_t start_index = -1)
    {
        std::vector<std::pair<E, std::string>> result_table;
        for (const auto& n : names_table)
        {
            result_table.emplace_back(std::make_pair(static_cast<E>(start_index)
                                                     , n));
            start_index++;
        }

        return result_table;
    }

    template<typename E>
    enum_converter<E>& get_converter(const std::vector<std::pair<E, std::string>>& conversion_table)
    {
        static enum_converter<E> converter(conversion_table);
        return converter;
    }

    template<typename E>
    enum_converter<E>& get_converter(std::int32_t start_index, const std::vector<std::string>& conversion_table)
    {
        static enum_converter<E> converter(create_consistent_table<E>(conversion_table
                                                                      , start_index));
        return converter;
    }
}

#define declare_enum_converter_begin(enum_type)\
    namespace __##enum_type \
    {\
        static auto __converter = detail::get_converter<enum_type>({
#define declare_consistent_enum_converter_begin(enum_type, start_index)\
    namespace __##enum_type \
    {\
        static auto __converter = detail::get_converter<enum_type>(start_index, {
#define declare_enum_converter_end(enum_type)\
        });\
    }\
    template<> bool convert<>(const enum_type& in_value, std::string& out_value) { return __##enum_type::__converter.convert(in_value, out_value); };\
    template<> bool convert<>(const std::string& in_value, enum_type& out_value) { return __##enum_type::__converter.convert(in_value, out_value); };\
    template std::string enum_to_string<enum_type>(const enum_type& enum_value, const std::string& default_string);\
    template enum_type string_to_enum<enum_type>(const std::string& enum_string, const enum_type& default_value);\
    template std::optional<enum_type> string_to_enum<enum_type>(const std::string& enum_string);\


template<typename E>
std::string enum_to_string(const E& enum_value, const std::string& default_string)
{
    std::string result(default_string);
    convert<E, std::string>(enum_value, result);
    return result;
}

template<typename E>
E string_to_enum(const std::string& enum_string, const E& default_value)
{
    E result = default_value;
    convert<std::string, E>(enum_string, result);
    return result;
}

template<typename E>
std::optional<E> string_to_enum(const std::string& enum_string)
{
    E result = {};
    if (convert<std::string, E>(enum_string, result))
    {
        return result;
    }
    return {};
}


declare_enum_converter_begin(device_type_t)
    std::make_pair(device_type_t::undefined, "undefined"),
    std::make_pair(device_type_t::v4l2, "v4l2"),
    std::make_pair(device_type_t::file, "file"),
    std::make_pair(device_type_t::http, "http"),
    std::make_pair(device_type_t::rtsp, "rtsp"),
    std::make_pair(device_type_t::rtmp, "rtmp"),
    std::make_pair(device_type_t::vnc, "vnc")
declare_enum_converter_end(device_type_t)

}
