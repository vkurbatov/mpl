#include "string_base.h"
#include <cstdarg>
#include <sstream>
#include <algorithm>

namespace base
{

namespace detail
{
    template<typename Func>
    void found_lines(const std::string &input
                     , const std::string& delimiter
                     , Func&& found_func
                     , std::size_t max_split = unlimited)
    {
        std::size_t start = 0;
        std::size_t end = 0;
        std::size_t delim_len = delimiter.length();

        if (!delimiter.empty())
        {
            while ((end = input.find(delimiter, start)) != std::string::npos
                   && max_split > 0)
            {
                auto token = input.substr(start, end - start);
                start = end + delim_len;
                if (!token.empty())
                {
                    found_func(std::move(token));
                    max_split--;
                }
            }
        }
        if (start < input.size())
        {
            found_func(input.substr(start));
        }
    }
}

std::string hex_dump(const void *dump, std::size_t size)
{
    std::string hex_string;

    hex_string.reserve(size * 3 + 1);

    auto data = static_cast<const std::uint8_t*>(dump);

    char hex[4] = {};

    while (size-- > 0)
    {
        if (!hex_string.empty())
        {
            hex_string.append(" ");
        }

        std::sprintf(hex, "%02x", *data);
        hex_string.append(hex);

        data++;
    }

    return hex_string;
}

std::vector<uint8_t> from_hex(const std::string &hex_string)
{
    std::vector<uint8_t> bytes;

    for (unsigned int i = 0; i < hex_string.length(); i += 2)
    {
        std::string byteString = hex_string.substr(i, 2);
        auto byte = static_cast<uint8_t>(strtol(byteString.c_str(), nullptr, 16));
        bytes.push_back(byte);
    }

    return bytes;
}

std::vector<std::string> split_string(const std::string &string
                                      , const std::string& delimiters)
{
    std::vector<std::string> split_list;

    std::string::size_type begin = 0;
    std::string::size_type end = std::string::npos;

    for (const auto& c : delimiters)
    {
        end = string.find(c, begin);

        if (end != std::string::npos)
        {
            split_list.push_back(string.substr(begin, end - begin));
            begin = end + 1;
        }
        else
        {
            split_list.push_back("");
        }
    }

    if (end != std::string::npos)
    {
        split_list.push_back(string.substr(begin, std::string::npos));
    }

    return split_list;
}

std::string format_string(const std::string &format, ...)
{
    char buffer[1024];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, sizeof(buffer), format.c_str(), args);

    va_end(args);

    return buffer;

}

std::vector<std::string> split_strings(const std::string &string, const std::string &delimiters)
{
    std::vector<std::string> split_list;

    std::string::size_type begin = 0;
    std::string::size_type end = std::string::npos;

    auto find_sep = [&]()
    {
        std::string::size_type result = std::string::npos;

        for (const auto& c : delimiters)
        {
            auto pos = string.find(c, begin);

            if (pos < result)
            {
                result = pos;
            }
        }

        return result;
    };

    while(begin != std::string::npos)
    {
        end = find_sep();

        if (end != std::string::npos)
        {
            split_list.push_back(string.substr(begin, end - begin));
            begin = end + 1;
        }
        else
        {
            split_list.push_back(string.substr(begin, std::string::npos));
            break;
        }
    }

    return split_list;
}

std::string lower_string(const std::string_view &string)
{
    std::string lower_string(string);
    std::transform(lower_string.begin(), lower_string.end(), lower_string.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return lower_string;
}

std::string upper_string(const std::string_view &string)
{
    std::string upper_string(string);
    std::transform(upper_string.begin(), upper_string.end(), upper_string.begin(),
        [](unsigned char c){ return std::tolower(c); });
    return upper_string;
}

int32_t check_sum(const void *dump, std::size_t size)
{
    int32_t cs = 0;

    while(size-- > 0)
    {
        cs += static_cast<const std::uint8_t*>(dump)[size];
    }

    return cs;
}

string_list_t split_lines(const std::string &input
                          , const std::string &delimiter
                          , std::size_t max_split)
{
    if (delimiter.empty() || delimiter == "\0")
    {
        return { input };
    }

    string_list_t split_list;
    auto found_func = [&split_list](std::string&& line)
    {
        split_list.emplace_back(line);
    };

    detail::found_lines(input
                        , delimiter
                        , std::move(found_func)
                        , max_split);

    return split_list;
}

string_list_t split_lines(const std::string &input
                          , char delimiter
                          , std::size_t max_split)
{
    if (max_split == 0 || delimiter == '\0')
    {
        return { input };
    }

    string_list_t split_list;
    std::istringstream stream(input);

    for(std::string line; std::getline(stream, line, max_split > 0 ? delimiter : '\0');)
    {
        if (delimiter == '\n'
                && !line.empty()
                && line.back() == '\r')
        {
            line.erase(std::prev(line.end()));
        }
        split_list.push_back(line);
        if (max_split == 0)
        {
            break;
        }
        max_split--;
    }

    return split_list;
}

std::string erase_substings(const std::string &input, const std::string &substring, std::size_t max_erase)
{
    if (!substring.empty())
    {
        std::string output;
        auto found_func = [&output](std::string&& line)
        {
            output.append(line);
        };

        detail::found_lines(input
                            , substring
                            , std::move(found_func)
                            , max_erase);

        return output;
    }

    return input;
}

string_param_list_t split_params(const std::string &input
                                 , const std::string &delimiter
                                 , const std::string &param_delimiter
                                 , std::size_t max_split)
{
    string_param_list_t param_list;

    for (auto& l : split_lines(input, delimiter, max_split))
    {
        auto key_value = split_lines(l, param_delimiter, 1);
        switch(key_value.size())
        {
            case 1:
                param_list.emplace_back(std::move(key_value[0])
                        , std::string{});
            break;
            case 2:
                param_list.emplace_back(std::move(key_value[0])
                        , std::move(key_value[1]));
            break;
        }
    }

    return param_list;
}

std::string build_string(const string_list_t &string_list
                         , const std::string &delimiter)
{
    std::string result;

    for (const auto& s : string_list)
    {
        if (!result.empty())
        {
            result.append(delimiter);
        }

        result.append(s);
    }

    return result;
}

std::string build_string(const string_param_list_t &param_list
                         , const std::string &delimiter
                         , const std::string &param_delimiter)
{
    std::string result;

    for (const auto& s : param_list)
    {
        if (!result.empty())
        {
            result.append(delimiter);
        }

        result.append(s.first);
        if (!s.second.empty())
        {
            result.append(param_delimiter);
            result.append(s.second);
        }
    }

    return result;
}

bool is_equal(const std::string &lstr
              , const std::string &rstr
              , bool case_sensetive)
{
    if (case_sensetive)
    {
        if (lstr.size() == rstr.size())
        {
            std::size_t n = 0;
            for (const auto& c : lstr)
            {
                if (std::tolower(c) != std::tolower(rstr[n++]))
                {
                    return false;
                }
            }
            return true;
        }
        return false;
    }

    return lstr == rstr;
}

bool compare(const std::string &text, const std::string &filter)
{
    string_list_t filter_params;

    for (const auto& f : filter)
    {
        if (filter_params.empty())
        {
            filter_params.emplace_back(std::string({f}));
        }
        else
        {
            const char& c = filter_params.back().back();
            switch(f)
            {
                case '?':
                {
                    if (c == f)
                    {
                        filter_params.back().push_back(f);
                    }
                    else
                    {
                        filter_params.push_back({f});
                    }
                }
                break;
                case '*':
                {
                    if (c != f)
                    {
                        filter_params.push_back({f});
                    }
                }
                break;
                default:
                {
                    if (c != '*'
                            && c != '?')
                    {
                        filter_params.back().push_back(f);
                    }
                    else
                    {
                        filter_params.push_back({f});
                    }
                }
            }
        }
    }

    std::string::size_type idx = 0;

    bool star = false;
    bool next = true;

    for (const auto& p : filter_params)
    {
        if (!next)
        {
            return false;
        }

        const char& c = p.front();
        switch(c)
        {
            case '?':
                idx += p.size();
            break;
            case '*':
               // nothing
            break;
            default:
            {
                auto pos = text.find(p, idx);
                bool found = pos != std::string::npos;
                if (found)
                {
                    if (star)
                    {
                        idx = pos + p.size();
                    }
                    else
                    {
                        if (idx == pos)
                        {
                            idx += p.size();
                        }
                        else
                        {
                            found = false;
                        }
                    }
                }
                next = found;
            }
        }
        star = c == '*';
        next &= idx <= text.size();
    }

    return star || idx == text.size();
}

std::string hex_to_string(const void *hex_data
                          , std::size_t hex_size
                          , const std::string &delimiter
                          , bool upper_case)
{
    std::string hex_string;

    const auto* ptr = static_cast<const std::uint8_t*>(hex_data);
    const auto* end = ptr + hex_size;
    char hex[4];
    static const char* hex_format[] = { "%02x", "%02X"};
    while (ptr < end)
    {
        if (!hex_string.empty())
        {
            hex_string.append(delimiter);
        }

        std::sprintf(hex, hex_format[static_cast<std::int32_t>(upper_case)], *ptr++);

        hex_string.append(hex);
    }

    return hex_string;
}



hex_dump_t string_to_hex(const std::string &hex_string
                         , const std::string &delimiter)
{
    hex_dump_t hex_dump;
    try
    {
        if (delimiter.empty())
        {
            auto string_size = hex_string.size() - hex_string.size() % 2;
            for (std::size_t i = 0; i < string_size; i += 2)
            {
                hex_dump.push_back(std::stoul(hex_string.substr(i, 2), nullptr, 16));
            }
        }
        else
        {
            for (const auto& h : split_lines(hex_string, delimiter))
            {
                hex_dump.push_back(std::stoul(h, nullptr, 16));
            }
        }
    }
    catch(const std::exception& e)
    {
        hex_dump.clear();
    }

    return hex_dump;
}

}
