#ifndef MPL_UTILS_H
#define MPL_UTILS_H

#include "core/property_types.h"
#include "core/common_types.h"

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

raw_array_t create_raw_array(const std::string_view& hex_string
                             , const std::string_view& delimeter = {});

template<typename T>
property_type_t get_property_type();

template<typename T>
std::size_t get_value_size(const T& value);

template<typename T>
T random();

template<typename T>
std::vector<T> random_array(std::size_t len);

const std::string& get_alphabet();
std::string random_string(std::size_t len, const std::string& alphabet = get_alphabet());


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
std::string& replace(std::string& input
                     , const std::string_view& from
                     , const std::string_view& to);

std::string to_lower(const std::string& input);
std::string to_upper(const std::string& input);
std::string replace(const std::string_view& input
                     , const std::string_view& from
                     , const std::string_view& to);


bool is_equal(const std::string& lstr
              , const std::string& rstr
              , bool case_sensetive);

bool compare(const std::string& text
             , const std::string& filter);

std::string hex_to_string(const void* data
                          , std::size_t size
                          , const std::string_view& delimiter = {}
                          , bool upper_case = false);

raw_array_t string_to_hex(const std::string_view& hex_string = {}
                          , const std::string_view& delimiter = {});

}

#endif // MPL_UTILS_H
