#ifndef STRING_BASE_H
#define STRING_BASE_H

#include <string>
#include <vector>

namespace portable
{


constexpr std::size_t unlimited = UINT64_MAX;

using hex_dump_t = std::vector<std::uint8_t>;
using string_list_t = std::vector<std::string>;
using string_param_t = std::pair<std::string, std::string>;
using string_param_list_t = std::vector<string_param_t>;

std::string hex_dump(const void *dump
                      , std::size_t size);

std::vector<std::uint8_t> from_hex(const std::string& hex_string);

std::int32_t check_sum(const void *dump
                       , std::size_t size);

std::vector<std::string> split_string(const std::string& string
                                      , const std::string& delimiters);

std::vector<std::string> split_strings(const std::string& string
                                       , const std::string& delimiters);

string_list_t split_lines(const std::string &input
                          , const std::string& delimiter
                          , std::size_t max_split = unlimited);

string_list_t split_lines(const std::string& input
                          , char delimiter = '\n'
                          , std::size_t max_split = unlimited);

std::string erase_substings(const std::string& input
                            , const std::string& substring
                            , std::size_t max_erase = unlimited);

string_param_list_t split_params(const std::string& input
                                , const std::string& delimiter
                                , const std::string& param_delimiter
                                , std::size_t max_split = unlimited);

std::string build_string(const string_list_t& string_list
                         , const std::string& delimiter);

std::string build_string(const string_param_list_t& param_list
                         , const std::string& delimiter
                         , const std::string& param_delimiter);

bool is_equal(const std::string& lstr
              , const std::string& rstr
              , bool case_sensetive);

std::string format_string(const std::string& format, ...);

std::string lower_string(const std::string_view& string);
std::string upper_string(const std::string_view& string);

bool compare(const std::string& text
             , const std::string& filter);

std::string hex_to_string(const void* hex_data
                          , std::size_t hex_size
                          , const std::string_view& delimiter = {}
                          , bool upper_case = false);

hex_dump_t string_to_hex(const std::string_view& hex_string
                         , const std::string_view& delimiter);

}

#endif // STRING_BASE_H
