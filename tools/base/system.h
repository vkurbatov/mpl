#ifndef SYSTEM_H
#define SYSTEM_H

#include <cstdint>

namespace base
{

double cpu_usage();
std::int64_t pid();
std::size_t get_open_handles(std::int64_t pid = pid());

}

#endif // SYSTEM_H
