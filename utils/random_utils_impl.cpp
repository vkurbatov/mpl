#include "common_utils.h"

#include "tools/base/random_base.h"

namespace mpl::utils
{


#define __declare_template_random_function(type)\
    template<>\
    type random() { return portable::utils::random<type>(); }\
    template std::vector<type> random_array(std::size_t len);

__declare_template_random_function(std::int8_t)
__declare_template_random_function(std::int16_t)
__declare_template_random_function(std::int32_t)
__declare_template_random_function(std::int64_t)
__declare_template_random_function(std::uint8_t)
__declare_template_random_function(std::uint16_t)
__declare_template_random_function(std::uint32_t)
__declare_template_random_function(std::uint64_t)

template<typename T>
std::vector<T> random_array(std::size_t len)
{
    std::vector<T> array(len);
    for (auto& v : array)
    {
        v = random<T>();
    }

    return array;
}

const std::string &get_alphabet()
{
    static std::string default_alphabet = "0123456789"
                                          "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                          "abcdefghijklmnopqrstuvwxyz";

    return default_alphabet;
}


std::string random_string(std::size_t len
                          , const std::string &alphabet)
{
    return portable::utils::random_string(len, alphabet);
}

}
