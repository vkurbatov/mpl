#include "option_impl.h"

namespace mpl
{

option_impl::u_ptr_t option_impl::create(const option_map_t &options)
{
    return std::make_unique<option_impl>(options);
}

option_impl::u_ptr_t option_impl::create(option_map_t &&options)
{
    return std::make_unique<option_impl>(std::move(options));
}

option_impl::u_ptr_t option_impl::create(const i_option &options)
{
    return std::make_unique<option_impl>(options);
}

option_impl::option_impl(const option_map_t &options)
    : m_options(options)
{

}

option_impl::option_impl(option_map_t &&options)
    : m_options(std::move(options))
{

}

option_impl::option_impl(const i_option &options)
{
    option_impl::merge(options);
}

option_impl &option_impl::assign(const i_option &options)
{
    m_options.clear();
    option_impl::merge(options);
    return *this;
}

option_impl::option_map_t option_impl::release()
{
    return std::move(m_options);
}

bool option_impl::has_option(const option_key_t &key) const
{
    return m_options.find(key) != m_options.end();
}

bool option_impl::set(const option_key_t &key
                      , option_value_t &&value)
{
    if (value.has_value())
    {
        return m_options.emplace(key
                                 , std::move(value)).second;
    }
    else
    {
        return m_options.erase(key) > 0;
    }

    return false;
}

bool option_impl::set(const option_key_t &key
                      , const option_value_t &value)
{
    return option_impl::set(key
                            , option_value_t(value));
}

i_option::option_value_t option_impl::get(const option_key_t &key) const
{
    auto it = m_options.find(key);
    return it != m_options.end()
            ? it->second
            : option_value_t{};
}

i_option::option_key_list_t option_impl::keys() const
{
    i_option::option_key_list_t keys;

    for (const auto& o : m_options)
    {
        keys.push_back(o.first);
    }

    std::sort(keys.begin(), keys.end());

    return keys;
}

void option_impl::foreach(const foreach_handler_t &handler) const
{
    if (handler)
    {
        for (const auto& o : m_options)
        {
            if (!handler(o.first, o.second))
            {
                break;
            }
        }
    }
}

void option_impl::clear()
{
    m_options.clear();
}

std::size_t option_impl::merge(const i_option &other)
{
    std::size_t counter = 0;
    other.foreach([&](const auto& key, const auto& value)
    {
        if (set(key, value))
        {
            counter++;
        }
        return true;
    });

    return counter;
}

bool option_impl::is_equal(const i_option &other) const
{
    if (keys() == other.keys())
    {
        for (const auto& o : m_options)
        {
            if (o.second != other.get(o.first))
            {
                return false;
            }
        }
        return true;
    }

    return false;
}

std::size_t option_impl::count() const
{
    return m_options.size();
}

i_option::u_ptr_t option_impl::clone() const
{
    return create(m_options);
}

}
