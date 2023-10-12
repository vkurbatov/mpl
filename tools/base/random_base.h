#ifndef BASE_RANDOM_BASE_H
#define BASE_RANDOM_BASE_H

#include <cstdint>
#include <string>

namespace portable::utils
{

void random(void* data, std::size_t size);
std::string random_string(std::size_t len, const std::string& alphabet);

template<typename T>
T random()
{
    T value = {};
    random(&value, sizeof(value));
    return value;
}

}

#endif // BASE_RANDOM_BASE_H
