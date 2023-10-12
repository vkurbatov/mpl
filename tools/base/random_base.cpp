#include "random_base.h"
#include <algorithm>
#include <ctime>

namespace portable::utils
{

void random(void *data, std::size_t size)
{
    static bool rand_init = false;
    if (rand_init == false)
    {
        std::srand(time(nullptr));
        rand_init = true;
    }

    auto ptr = static_cast<std::uint8_t*>(data);
    auto end = ptr + size;

    while (ptr < end)
    {
        auto rand_value = std::rand();
        if ((end - ptr) == 1)
        {
            *reinterpret_cast<uint8_t*>(ptr) = *reinterpret_cast<const uint8_t*>(&rand_value);
            break;
        }
        *reinterpret_cast<uint16_t*>(ptr) = *reinterpret_cast<const uint16_t*>(&rand_value);
        ptr += 2;
    }
}

std::string random_string(std::size_t len, const std::string &alphabet)
{
    std::string result(len, 0);
    for (std::size_t i = 0; i < len; i++)
    {
        result[i] = alphabet[random<std::uint16_t>() % alphabet.size()];
    }

    return result;
}


}
