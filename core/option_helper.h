#ifndef MPL_OPTION_HELPER_H
#define MPL_OPTION_HELPER_H

#include "i_option.h"
#include "option_value_impl.h"
#include <optional>

namespace mpl
{

class option_reader
{
    const i_option& m_options;
public:

    template<typename T, typename CT = i_option_canonical_value<T>>
    static const CT* cast(const i_option_value& value)
    {
        if (value.type_info().type_id == type_info_t::get_type_info<T>().type_id)
        {
            return static_cast<const CT*>(&value);
        }

        return nullptr;
    }

    template<typename T, typename CT = i_option_canonical_value<T>>
    static CT* cast(i_option_value& value)
    {
        if (value.type_info().type_id == type_info_t::get_type_info<T>().type_id)
        {
            return static_cast<CT*>(&value);
        }

        return nullptr;
    }

    option_reader(const i_option& options);

    template<typename T>
    bool has_type(const i_option::option_id_t& key) const
    {
        auto any_option_value = m_options.get(key);
        return any_option_value != nullptr
                && cast<T>(*any_option_value) != nullptr;
    }

    template<typename T>
    bool get(const i_option::option_id_t& key, T& value) const
    {
        if (auto any_option_value = m_options.get(key))
        {
            if (auto option_value = cast<T>(*any_option_value))
            {
                value = option_value->get();
                return true;
            }
        }

        return false;
    }

    template<typename T>
    const T& get(const i_option::option_id_t& key, const T& default_value) const
    {
        if (auto any_option_value = m_options.get(key))
        {
            if (auto option_value = cast<T>(*any_option_value))
            {
                return option_value->get();
            }
        }

        return default_value;
    }

    template<typename T, typename VT = std::enable_if_t<!std::is_pointer_v<T>>>
    std::optional<T> get(const i_option::option_id_t& key) const
    {
        if (auto any_option_value = m_options.get(key))
        {
            if (auto option_value = cast<T>(*any_option_value))
            {
                return option_value->get();
            }
        }
        return std::nullopt;
    }

    template<typename T
             , typename VT = std::enable_if_t<std::is_pointer_v<T>>
             , typename CT = std::remove_pointer_t<T>>
    const CT* get(const i_option::option_id_t& key) const
    {
        if (const auto any_option_value = m_options.get(key))
        {
            if (auto option_value = cast<CT>(*any_option_value))
            {
                return &option_value->get();
            }
        }
        return nullptr;
    }

};


class option_writer : public option_reader
{
    i_option&   m_options;
public:
    option_writer(i_option& options);

    template<typename T, typename VT = std::decay_t<T>>
    bool set(const i_option::option_id_t& id, const T& value)
    {
        return m_options.set(id
                             , option_value_impl<VT>::create(value));
    }
    template<typename T, typename VT = std::decay_t<T>>
    bool set(const i_option::option_id_t& id, T&& value)
    {
        return m_options.set(id
                             , option_value_impl<VT>::create(std::move(value)));

    }

    template<typename T, class... Args>
    bool set(const i_option::option_id_t& key, Args&& ...args)
    {
        return m_options.set(key
                             , option_value_impl<std::decay_t<T>>::create(T(args...)));
    }

    bool remove(const i_option::option_id_t& key);
};

}

#endif // MPL_OPTION_HELPER_H
