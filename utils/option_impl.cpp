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

option_impl::option_map_t option_impl::clone_options(const option_map_t &options)
{
    option_impl::option_map_t clone_options;
    for (const auto& o : options)
    {
        clone_options.emplace(o.first
                              , o.second->clone());

    }

    return clone_options;
}

option_impl::u_ptr_t option_impl::create(const i_option &options)
{
    return std::make_unique<option_impl>(options);
}

option_impl::option_impl(const option_impl &option)
    : option_impl(option.m_options)
{

}

option_impl::option_impl(const option_map_t &options)
     : m_options(clone_options(options))
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

option_impl &option_impl::operator=(const option_impl &option)
{
    m_options = clone_options(option.m_options);
    return *this;
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

bool option_impl::has_option(const option_id_t &id) const
{
    return m_options.find(id) != m_options.end();
}

bool option_impl::set(const option_id_t &id
                      , i_option_value::u_ptr_t &&value)
{
    if (value)
    {
        m_options[id] = std::move(value);
        return true;
    }
    else
    {
        return m_options.erase(id) > 0;
    }

    return false;
}

bool option_impl::set(const option_id_t &id
                      , const i_option_value &value)
{

    return option_impl::set(id
                            , value.clone());
}

const i_option_value* option_impl::get(const option_id_t &id) const
{
    const auto& it = m_options.find(id);
    return it != m_options.end()
            ? it->second.get()
            : nullptr;
}

i_option::option_id_list_t option_impl::ids() const
{
    i_option::option_id_list_t ids;

    for (const auto& o : m_options)
    {
        ids.push_back(o.first);
    }

    std::sort(ids.begin(), ids.end());

    return ids;
}

void option_impl::foreach(const foreach_handler_t &handler) const
{
    if (handler)
    {
        for (const auto& o : m_options)
        {
            if (o.second)
            {
                if (!handler(o.first, *o.second))
                {
                    break;
                }
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
    other.foreach([&](const auto& id, const auto& value)
    {
        if (set(id, value))
        {
            counter++;
        }
        return true;
    });

    return counter;
}

bool option_impl::is_equal(const i_option &other) const
{
    if (ids() == other.ids())
    {
        for (const auto& o : m_options)
        {
            auto l = o.second.get();
            auto r = other.get(o.first);
            if (l != r
                    || !l->is_equal(*r))
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
