#ifndef MPL_I_MODULE_H
#define MPL_I_MODULE_H

#include "module_types.h"
#include <memory>

namespace mpl
{

class i_module
{
public:
    using u_ptr_t = std::unique_ptr<i_module>;
    using s_ptr_t = std::shared_ptr<i_module>;

    virtual ~i_module() = default;
    virtual module_id_t module_id() const = 0;

};

}

#endif // MPL_I_MODULE_H
