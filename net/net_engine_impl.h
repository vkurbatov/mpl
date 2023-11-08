#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"

namespace mpl
{

class i_task_manager;
class i_timer_manager;

namespace net
{

struct net_engine_config_t;

class net_engine_factory
{
public:

    static net_engine_factory& get_instance();

    i_net_engine::u_ptr_t create_engine(const net_engine_config_t& config
                                        , i_task_manager& task_manager
                                        , i_timer_manager& timer_manager);
};

}

}

#endif // MPL_NET_ENGINE_IMPL_H
