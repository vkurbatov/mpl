#ifndef MPL_POINTER_UTILS_H
#define MPL_POINTER_UTILS_H

#include <memory>

namespace mpl::utils
{

template<typename T, typename U>
std::unique_ptr<T> static_pointer_cast(std::unique_ptr<U>&& ptr)
{
    return std::unique_ptr<T>(static_cast<T*>(ptr.release()));
}

template<typename T, typename U>
std::unique_ptr<T> dynamic_pointer_cast(std::unique_ptr<U>&& ptr)
{
    if (auto p = dynamic_cast<T*>(ptr.get()))
    {
        ptr.release();
        return std::unique_ptr<T>(p);
    }

    return nullptr;
}

}

#endif // MPL_POINTER_UTILS_H
