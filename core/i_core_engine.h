#ifndef MPL_I_CORE_ENGINE_H
#define MPL_I_CORE_ENGINE_H

#include "i_engine.h"

namespace mpl
{

class i_task_manager;
class i_timer_manager;
class i_buffer_factory;

class i_core_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_core_engine>;
    using s_ptr_t = std::shared_ptr<i_core_engine>;

    virtual i_task_manager& task_manager() = 0;
    virtual i_timer_manager& timer_manager() = 0;
    virtual i_buffer_factory& buffer_factory() = 0;

};

}

#endif // MPL_I_CORE_ENGINE_H
