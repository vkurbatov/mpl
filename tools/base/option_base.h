#ifndef OPTION_BASE_H
#define OPTION_BASE_H

#include <vector>
#include <string>

namespace base
{

typedef std::pair<std::string, std::string> option_t;
typedef std::vector<option_t> option_list_t;

option_list_t parse_option_list(const std::string& options_string);

}

#endif // OPTION_BASE_H
