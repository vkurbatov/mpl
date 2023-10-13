#ifndef SSL_POINTERS_H
#define SSL_POINTERS_H

#include <memory>

namespace
{

template<typename T>
using ssl_unique_deleter_t = void(*)(T*);

template<typename T>
using ssl_s_ptr_t = std::shared_ptr<T>;

template<typename T>
using ssl_unique_s_ptr_t = std::unique_ptr<T, ssl_unique_deleter_t<T>>;

}

#endif // SSL_POINTERS_H
