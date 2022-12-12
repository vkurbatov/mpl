#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>

namespace base
{

typedef std::vector<std::string> file_list_t;

file_list_t get_files(const std::string& dir_name);
std::string extract_file_extension(const std::string& file_name);
std::string get_current_directory();
bool set_current_directory(const std::string& dir_name);

}

#endif // FILESYSTEM_H
