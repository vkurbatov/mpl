#include "property_tree_impl.h"
#include "utils.h"

namespace mpl
{

property_tree::u_ptr_t property_tree::create(const property_map_t& propertyes)
{
    return std::make_unique<property_tree>(propertyes);
}

property_tree::u_ptr_t property_tree::create(property_map_t&& propertyes)
{
    return std::make_unique<property_tree>(std::move(propertyes));
}

property_tree::property_tree(const property_map_t& propertyes)
    : m_propertyes(propertyes)
{

}

property_tree::property_tree(property_tree::property_map_t&& propertyes)
    : m_propertyes(std::move(propertyes))
{

}

property_type_t property_tree::property_type() const
{
    return property_type_t::object;
}

std::size_t property_tree::size() const
{
    return m_propertyes.size();
}

i_property::u_ptr_t property_tree::clone() const
{
    if (auto clone_tree = create())
    {
        clone_tree->merge(*this);
        return clone_tree;
    }

    return nullptr;
}

bool property_tree::merge(const i_property &property)
{
    bool result = false;
    if (property.property_type() == property_type_t::object)
    {
        const auto& tree = static_cast<const i_property_tree&>(property);
        for (const auto& k : tree.property_list())
        {
            if (auto p = tree.property(k))
            {
                auto it = m_propertyes.find(k);
                if (it == m_propertyes.end()
                        || it->second == nullptr)
                {
                    m_propertyes.emplace(k
                                         , std::move(p->clone()));
                    result = true;
                }
                else
                {
                    result |= it->second->merge(*p);
                }
            }
        }
    }

    return result;
}

void property_tree::clear()
{
    m_propertyes.clear();
}

bool property_tree::is_equal(const i_property &property) const
{
    if (property.property_type() == property_type_t::object)
    {
        const auto& tree = static_cast<const i_property_tree&>(property);
        if (property_list() == tree.property_list())
        {
            for (const auto& pair : m_propertyes)
            {
                auto p = tree.property(pair.first);
                if (p == pair.second.get())
                {
                    continue;
                }

                if (p == nullptr
                       || pair.second == nullptr
                       || !pair.second->is_equal(*p))
                {
                    return false;
                }
            }
            return true;
        }
    }

    return false;
}

bool property_tree::set(const value_type_t &key, const i_property::s_ptr_t& property)
{
    auto args = utils::split_lines(key
                                   , '.'
                                   , 1);
    if (args.size() > 0)
    {
        if (args.size() > 1)
        {
            auto it = m_propertyes.find(args[0]);
            if (it != m_propertyes.end()
                    && it->second != nullptr)
            {
                if (it->second->property_type() == property_type_t::object)
                {
                    auto& tree = static_cast<i_property_tree&>(*it->second);
                    tree.set(args[1], std::move(property));
                    return true;
                }
            }
            auto tree = create();
            tree->set(args[1], std::move(property));
            m_propertyes[args[0]] = std::move(tree);

            return true;
        }
        else
        {
            if (property == nullptr)
            {
                return m_propertyes.erase(args[0]) > 0;
            }
            else
            {
                if (property.use_count() > 1)
                {
                    m_propertyes[args[0]] = property->clone();
                }
                else
                {
                    m_propertyes[args[0]] = std::move(property);
                }

                return true;
            }
        }
    }

    return false;
}

i_property::u_ptr_t property_tree::get(const value_type_t &key) const
{
    if (auto propertry = find_property(key))
    {
        return propertry->clone();
    }

    return nullptr;
}

const i_property *property_tree::property(const value_type_t &key) const
{
    return find_property(key);
}

i_property *property_tree::property(const value_type_t &key)
{
    return find_property(key);
}

i_property_tree::property_list_t property_tree::property_list(bool recursion) const
{
    i_property_tree::property_list_t list;
    for (const auto& p : m_propertyes)
    {
        list.push_back(p.first);
        if (recursion
                && p.second->property_type() == property_type_t::object)
        {
            for (const auto& s : static_cast<const i_property_tree&>(*p.second).property_list(recursion))
            {
                list.emplace_back(std::string(p.first).append(".").append(s));
            }
        }
    }
    return list;
}

i_property *property_tree::find_property(const value_type_t &key)
{
    return const_cast<i_property*>(static_cast<const property_tree*>(this)->find_property(key));
}

const i_property* property_tree::find_property(const value_type_t &key) const
{
    auto args = utils::split_lines(key, '.', 1);
    if (args.size() > 0)
    {
        auto it = m_propertyes.find(args[0]);
        if (it != m_propertyes.end())
        {
            if (args.size() == 1)
            {
                return it->second.get();
            }
            else if (it->second
                     && it->second->property_type() == property_type_t::object)
            {
                return static_cast<i_property_tree&>(*it->second).property(args[1]);
            }
        }
    }

    return nullptr;
}



}
