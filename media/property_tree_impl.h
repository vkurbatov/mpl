#ifndef MPL_PROPERTY_TREE_IMPL_H
#define MPL_PROPERTY_TREE_IMPL_H

#include "i_property_tree.h"

#include <map>

namespace mpl
{

class property_tree : public i_property_tree
{
public:
    using property_map_t = std::map<value_type_t, i_property::s_ptr_t>;
private:
    property_map_t  m_propertyes;

public:
    using u_ptr_t = std::unique_ptr<property_tree>;
    using s_ptr_t = std::shared_ptr<property_tree>;

    static u_ptr_t create(const property_map_t& propertyes = {});
    static u_ptr_t create(property_map_t&& propertyes);

    property_tree(const property_map_t& propertyes = {});
    property_tree(property_map_t&& propertyes);

    // i_property interface
public:
    property_type_t property_type() const override;
    std::size_t size() const override;
    i_property::u_ptr_t clone() const override;
    bool merge(const i_property& property) override;
    void clear() override;
    bool is_equal(const i_property& property) const override;

    // i_property_tree interface
public:

    bool set(const value_type_t &key, const i_property::s_ptr_t& property) override;
    i_property::u_ptr_t get(const value_type_t &key) const override;
    const i_property *property(const value_type_t &key) const override;
    i_property *property(const value_type_t &key) override;

    property_list_t property_list(bool recursion = false) const override;

private:
    i_property* find_property(const value_type_t &key);
    const i_property* find_property(const value_type_t &key) const;


};

}

#endif // MPL_PROPERTY_TREE_IMPL_H
