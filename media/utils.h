#ifndef MPL_UTILS_H
#define MPL_UTILS_H

#include "property_types.h"
#include "common_types.h"

#include <string>
#include <cstdint>

namespace mpl::utils
{

namespace limits
{
constexpr std::size_t unlimited = UINT64_MAX;
}


using string_list_t = std::vector<std::string>;
using hex_dump_t = std::vector<std::uint8_t>;
using string_param_t = std::pair<std::string, std::string>;
using string_param_list_t = std::vector<string_param_t>;

raw_array_t create_raw_array(const void* ref_data
                             , std::size_t ref_size);

template<typename T>
property_type_t get_property_type();

template<typename T>
std::size_t get_value_size(const T& value);


// string utils
string_list_t split_lines(const std::string &input
                          , const std::string& delimiter
                          , std::size_t max_split = limits::unlimited);

string_list_t split_lines(const std::string& input
                          , char delimiter = '\n'
                          , std::size_t max_split = limits::unlimited);

std::string erase_substings(const std::string& input
                            , const std::string& substring
                            , std::size_t max_erase = limits::unlimited);

string_param_list_t split_params(const std::string& input
                                , const std::string& delimiter
                                , const std::string& param_delimiter
                                , std::size_t max_split = limits::unlimited);

std::string build_string(const string_list_t& string_list
                         , const std::string& delimiter);

std::string build_string(const string_param_list_t& param_list
                         , const std::string& delimiter
                         , const std::string& param_delimiter);

std::string& to_lower(std::string& input);
std::string& to_upper(std::string& input);

std::string to_lower(const std::string& input);
std::string to_upper(const std::string& input);

bool is_equal(const std::string& lstr
              , const std::string& rstr
              , bool case_sensetive);

bool compare(const std::string& text
             , const std::string& filter);

}

#endif // MPL_UTILS_H
