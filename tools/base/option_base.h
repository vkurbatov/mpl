#ifndef OPTION_BASE_H
#define OPTION_BASE_H

#include <vector>
#include <string>
#include <map>

namespace portable
{

using option_t = std::pair<std::string,std::string>;
using option_list_t = std::vector<option_t>;
using option_map_t = std::map<std::string,std::string>;

option_list_t parse_option_list(const std::string& options_string);
option_map_t parse_option_map(const std::string& options_string);


}

#endif // OPTION_BASE_H
