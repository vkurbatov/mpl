#ifndef MPL_I_NET_ENGINE_H
#define MPL_I_NET_ENGINE_H

#include "core/i_engine.h"
#include "i_net_packet_builder.h"
#include "net_types.h"

namespace mpl
{

class i_task_manager;
class i_timer_manager;

namespace net
{

class i_transport_factory;

class i_net_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_net_engine>;
    using s_ptr_t = std::shared_ptr<i_net_engine>;

    virtual i_task_manager& task_manager() = 0;
    virtual i_timer_manager& timer_manager() = 0;
    virtual i_transport_factory* transport_factory(transport_id_t transport_id) = 0;
    virtual i_net_packet_builder::u_ptr_t create_packet_builder(transport_id_t transport_id) = 0;

    // ???
};

}

}


#endif // MPL_I_NET_ENGINE_H
