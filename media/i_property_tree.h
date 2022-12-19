#ifndef MPL_I_PROPERTY_TREE_H
#define MPL_I_PROPERTY_TREE_H

#include "i_property.h"
#include <string>

namespace mpl
{

class i_property_tree : public i_property
{
public:
    using value_type_t = std::string;
    using property_list_t = std::vector<value_type_t>;
    using u_ptr_t = std::unique_ptr<i_property_tree>;
    using s_ptr_t = std::shared_ptr<i_property_tree>;
    virtual ~i_property_tree(){}

    virtual bool set(const value_type_t& key, const i_property::s_ptr_t& property) = 0;
    virtual i_property::u_ptr_t get(const value_type_t& key) const  = 0;

    virtual const i_property* property(const value_type_t& key) const = 0;
    virtual i_property* property(const value_type_t& key) = 0;

    virtual property_list_t property_list(bool recursion = false) const = 0;

    virtual void clear() = 0;
};

}

#endif // MPL_I_PROPERTY_TREE_H
