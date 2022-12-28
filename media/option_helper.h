#ifndef MPL_OPTION_HELPER_H
#define MPL_OPTION_HELPER_H

#include "i_option.h"
#include <optional>

namespace mpl
{

class option_reader
{
    const i_option& m_options;
public:
    option_reader(const i_option& options);

    template<typename T>
    bool has_type(const i_option::option_key_t& key) const
    {
        auto any_option_value = m_options.get(key);
        return any_option_value.has_value()
                && std::any_cast<T>(&any_option_value) != nullptr;
    }

    template<typename T>
    bool get(const i_option::option_key_t& key, T& value) const
    {
        if (auto any_option_value = m_options.get(key))
        {
            if (auto option_value = any_option_value.cast<T*>())
            {
                value = *option_value;
                return true;
            }
        }

        return false;
    }

    template<typename T>
    std::optional<T> get(const i_option::option_key_t& key) const
    {
        if (auto any_option_value = m_options.get(key))
        {
            if (auto option_value = any_option_value.cast<T*>())
            {
                return *option_value;
            }
        }
        return std::nullopt;
    }

};


class option_writer : public option_reader
{
    i_option&   m_options;
public:
    option_writer(i_option& options);

    template<typename T>
    bool set(const i_option::option_key_t& key, const T& value)
    {
        return m_options.set(key
                             , value);
    }

    template<typename T>
    bool set(const i_option::option_key_t& key, T&& value)
    {
        return m_options.set(key
                             , std::move(value));
    }

    template<typename T, class... Args>
    bool set(const i_option::option_key_t& key, Args&& ...args)
    {
        return m_options.set(key
                             , T(args...));
    }

    bool remove(const i_option::option_key_t& key);
};

}

#endif // MPL_OPTION_HELPER_H
