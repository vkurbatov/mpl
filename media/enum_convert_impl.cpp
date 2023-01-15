#include "convert_utils.h"
#include "utils.h"
#include "enum_utils.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"

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

#define xstr(s) str(s)
#define str(s) #s

#define declare_pair(type, value)\
    std::make_pair(type::value, #value)

declare_enum_converter_begin(device_type_t)
    std::make_pair(device_type_t::undefined, "undefined"),
    std::make_pair(device_type_t::v4l2, "v4l2"),
    std::make_pair(device_type_t::file, "file"),
    std::make_pair(device_type_t::http, "http"),
    std::make_pair(device_type_t::rtsp, "rtsp"),
    std::make_pair(device_type_t::rtmp, "rtmp"),
    std::make_pair(device_type_t::vnc, "vnc")
declare_enum_converter_end(device_type_t)

declare_enum_converter_begin(media_type_t)
    std::make_pair(media_type_t::undefined, "undefined"),
    std::make_pair(media_type_t::audio, "audio"),
    std::make_pair(media_type_t::video, "video"),
    std::make_pair(media_type_t::data, "data"),
    std::make_pair(media_type_t::custom, "custom"),
declare_enum_converter_end(media_type_t)

declare_enum_converter_begin(audio_format_id_t)
    std::make_pair(audio_format_id_t::undefined, "undefined"),
    std::make_pair(audio_format_id_t::pcm8, "pcm8"),
    std::make_pair(audio_format_id_t::pcm16, "pcm16"),
    std::make_pair(audio_format_id_t::pcm32, "pcm32"),
    std::make_pair(audio_format_id_t::float32, "float32"),
    std::make_pair(audio_format_id_t::float64, "float64"),
    std::make_pair(audio_format_id_t::pcm8p, "pcm8p"),
    std::make_pair(audio_format_id_t::pcm16p, "pcm16p"),
    std::make_pair(audio_format_id_t::pcm32p, "pcm32p"),
    std::make_pair(audio_format_id_t::float32p, "float32p"),
    std::make_pair(audio_format_id_t::float64p, "float64p"),
    std::make_pair(audio_format_id_t::pcma, "pcma"),
    std::make_pair(audio_format_id_t::pcmu, "pcmu"),
    std::make_pair(audio_format_id_t::opus, "opus"),
    std::make_pair(audio_format_id_t::aac, "aac")
declare_enum_converter_end(audio_format_id_t)

declare_enum_converter_begin(video_format_id_t)
    declare_pair(video_format_id_t, undefined),
    declare_pair(video_format_id_t, yuv420p),
    declare_pair(video_format_id_t, yuv422p),
    declare_pair(video_format_id_t, yuv444p),
    declare_pair(video_format_id_t, yuv411p),
    declare_pair(video_format_id_t, yuyv),
    declare_pair(video_format_id_t, uyvy),
    declare_pair(video_format_id_t, yuv410),
    declare_pair(video_format_id_t, nv12),
    declare_pair(video_format_id_t, nv21),
    declare_pair(video_format_id_t, nv16),
    declare_pair(video_format_id_t, bgr555),
    declare_pair(video_format_id_t, bgr555x),
    declare_pair(video_format_id_t, bgr565),
    declare_pair(video_format_id_t, bgr565x),
    declare_pair(video_format_id_t, rgb555),
    declare_pair(video_format_id_t, rgb555x),
    declare_pair(video_format_id_t, rgb565),
    declare_pair(video_format_id_t, rgb565x),
    declare_pair(video_format_id_t, bgr8),
    declare_pair(video_format_id_t, rgb8),
    declare_pair(video_format_id_t, bgr24),
    declare_pair(video_format_id_t, rgb24),
    declare_pair(video_format_id_t, bgr32),
    declare_pair(video_format_id_t, rgb32),
    declare_pair(video_format_id_t, abgr32),
    declare_pair(video_format_id_t, argb32),
    declare_pair(video_format_id_t, bgra32),
    declare_pair(video_format_id_t, rgba32),
    declare_pair(video_format_id_t, gray8),
    declare_pair(video_format_id_t, gray16),
    declare_pair(video_format_id_t, gray16x),
    declare_pair(video_format_id_t, sbggr8),
    declare_pair(video_format_id_t, sgbrg8),
    declare_pair(video_format_id_t, sgrbg8),
    declare_pair(video_format_id_t, srggb8),
    declare_pair(video_format_id_t, png),
    declare_pair(video_format_id_t, jpeg),
    declare_pair(video_format_id_t, mjpeg),
    declare_pair(video_format_id_t, gif),
    declare_pair(video_format_id_t, h265),
    declare_pair(video_format_id_t, h264),
    declare_pair(video_format_id_t, h263),
    declare_pair(video_format_id_t, h263p),
    declare_pair(video_format_id_t, h261),
    declare_pair(video_format_id_t, vp8),
    declare_pair(video_format_id_t, vp9),
    declare_pair(video_format_id_t, mpeg4),
    declare_pair(video_format_id_t, cpia)
declare_enum_converter_end(video_format_id_t)

}
