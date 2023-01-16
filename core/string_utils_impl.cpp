#include "utils.h"

#include "tools/base/string_base.h"

namespace mpl::core::utils
{

string_list_t split_lines(const std::string &input
                          , const std::string& delimiter
                          , std::size_t max_split)
{
    return base::split_lines(input
                              , delimiter
                              , max_split);
}

string_list_t split_lines(const std::string& input
                          , char delimiter
                          , std::size_t max_split)
{
    return base::split_lines(input
                              , delimiter
                              , max_split);
}

std::string erase_substings(const std::string& input
                            , const std::string& substring
                            , std::size_t max_erase)
{
    return base::erase_substings(input
                                 , substring
                                 , max_erase);
}

string_param_list_t split_params(const std::string& input
                                , const std::string& delimiter
                                , const std::string& param_delimiter
                                , std::size_t max_split)
{
    return base::split_params(input
                              , delimiter
                              , param_delimiter
                              , max_split);
}

std::string build_string(const string_list_t& string_list
                         , const std::string& delimiter)
{
    return base::build_string(string_list
                              , delimiter);
}

std::string build_string(const string_param_list_t& param_list
                         , const std::string& delimiter
                         , const std::string& param_delimiter)
{
    return base::build_string(param_list
                              , delimiter
                              , param_delimiter);
}

std::string& to_lower(std::string& input)
{
    for (auto& c : input)
    {
        c = std::tolower(c);
    }

    return input;
}
std::string& to_upper(std::string& input)
{
    for (auto& c : input)
    {
        c = std::toupper(c);
    }

    return input;
}

std::string to_lower(const std::string& input)
{
    auto copy_input = input;
    return to_lower(copy_input);
}
std::string to_upper(const std::string& input)
{
    auto copy_input = input;
    return to_upper(copy_input);
}

bool is_equal(const std::string& lstr
              , const std::string& rstr
              , bool case_sensetive)
{
    return base::is_equal(lstr
                          , rstr
                          , case_sensetive);
}


bool compare(const std::string& text
             , const std::string& filter)
{
    return base::compare(text
                         , filter);
}

}
