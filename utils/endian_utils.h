#ifndef MPL_UTILS_ENDIAN_UTILS_H
#define MPL_UTILS_ENDIAN_UTILS_H

#include <cstdint>

namespace mpl::utils::endian
{

namespace big
{

void* convert(void* value, std::size_t size);

template<typename T>
T convert(const T& value);

template<typename T>
T get_value(const void* data
            , std::size_t size = sizeof(T)
            , std::int32_t offset = 0);


template<typename T>
std::size_t set_value(const T& value
                      , void* data
                      , std::size_t size = sizeof(T)
                      , std::int32_t offset = 0);

}

namespace little
{

void* convert(void* value
              , std::size_t size);

template<typename T>
T get_value(const void* data
            , std::size_t size = sizeof(T)
            , std::int32_t offset = 0);

template<typename T>
T convert(const T& value);

template<typename T>
std::size_t set_value(const T& value
                       , void* data
                       , std::size_t size = sizeof(T)
                       , std::int32_t offset = 0);

}

}

#endif // MPL_UTILS_EENDIAN_UTILS_H
