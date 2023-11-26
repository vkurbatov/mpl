#ifndef MPL_PROPERTY_HELPER_H
#define MPL_PROPERTY_HELPER_H

#include "core/i_property.h"
#include <string>

namespace mpl
{

class property_helper
{
    const i_property&   m_property;
public:
    static i_property::u_ptr_t create_object();
    static i_property::u_ptr_t create_array();
    static i_property::u_ptr_t create_array(i_property::s_array_t&& values);

    property_helper(const i_property& property);

    const i_property* operator[](const std::string& key) const;
    property_type_t get_type(const std::string& key) const;
    bool has_property(const std::string& key) const;
    std::vector<std::string> property_list(const std::string& key = {}
                                           , bool recursion = false) const;
};

}

#endif // MPL_PROPERTY_HELPER_H
