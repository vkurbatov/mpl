#include "filesystem.h"

#include <dirent.h>
#include <unistd.h>

namespace pt::utils
{



file_list_t get_files(const std::string &dir_name)
{
    file_list_t file_list;
    auto dir = opendir(dir_name.c_str());

    if (dir != nullptr)
    {
        do
        {
            auto dir_entry = readdir(dir);

            if (dir_entry == nullptr)
            {
                break;
            }

            auto file_name = dir_name;

            if (!dir_name.empty()
                    && dir_name.back() != '/')
            {
                file_name.append("/");
            }

            file_name.append(dir_entry->d_name);
            file_list.emplace_back(file_name);
        }
        while(true);

        closedir(dir);
    }

    return file_list;
}

std::string extract_file_extension(const std::string &file_name)
{
    auto pos = file_name.find_last_of('.');

    if (pos != std::string::npos)
    {
        return file_name.substr(pos + 1);
    }

    return std::string();
}

std::string get_current_directory()
{
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr)
    {
        return std::string(cwd);
    }

    return {};
}

bool set_current_directory(const std::string& dir_name)
{
    return chdir(dir_name.c_str()) >= 0;
}

}
