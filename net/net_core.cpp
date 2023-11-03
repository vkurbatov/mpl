#include "net_core.h"

#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"

#include "net_engine_impl.h"
#include "net_engine_config.h"

namespace mpl::net
{

struct net_core_context_t
{
    net_engine_impl                 engine;

    net_core_context_t(const net_engine_config_t& config
                       , i_task_manager& task_manager
                       , i_timer_manager& timer_manager)
        : engine(config
                 , task_manager
                 , timer_manager)
    {

    }

    net_core_context_t(const net_engine_config_t& config)
        : net_core_context_t(config
                            , task_manager_factory::single_manager()
                            , timer_manager_factory::single_manager())
    {

    }
};

i_net_engine& net_core::get_engine(const net_engine_config_t &engine_config)
{
    static net_core_context_t core_context(engine_config);
    return core_context.engine;
}

}
