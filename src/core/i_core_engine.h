#ifndef MPL_I_CORE_ENGINE_H
#define MPL_I_CORE_ENGINE_H

#include "i_engine.h"
#include "i_core_module.h"

namespace mpl
{


class i_core_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_core_engine>;
    using s_ptr_t = std::shared_ptr<i_core_engine>;

    virtual i_core_module& core() = 0;
};

}

#endif // MPL_I_CORE_ENGINE_H
