#include "option_base.h"

namespace base
{

static void trim_spaces(std::string& string
                        , const std::string& whitespaces = " \t\f\v\n\r")
{
    if (!string.empty())
    {
        auto first = string.find_first_not_of(whitespaces);

        if (first != std::string::npos)
        {
            auto last = string.find_last_not_of(whitespaces);

            if (last != string.size())
            {
                string = string.substr(first
                                       , last - first + 1);
            }
        }
        else
        {
            string.clear();
        }
    }
}

option_list_t parse_option_list(const std::string& options_string)
{
    option_list_t options;

    if (!options_string.empty())
    {
        size_t start = 0, end = 0;

        while ((end = options_string.find('=', start))
               != std::string::npos)
        {
            option_t option;

            option.first = options_string.substr(start, end - start);

            start = end + 1;
            end = options_string.find(';', start);

            option.second = options_string.substr(start, end - start);

            trim_spaces(option.first);
            trim_spaces(option.second);

            if (!option.first.empty()
                    && !option.second.empty())
            {
                options.emplace_back(std::move(option));
            }

            if (end != std::string::npos)
            {
                start = end + 1;
            }
            else
            {
                break;
            }
        }
    }

    return options;
}

}
