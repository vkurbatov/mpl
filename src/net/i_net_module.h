#ifndef MPL_I_NET_MODULE_H
#define MPL_I_NET_MODULE_H

#include "core/i_module.h"
#include "i_net_packet_builder.h"
#include "net_types.h"

namespace mpl::net
{

class i_transport_factory;
class i_transport_collection;

class i_net_module : public i_module
{
public:
    using u_ptr_t = std::unique_ptr<i_net_module>;
    using s_ptr_t = std::shared_ptr<i_net_module>;

    virtual i_transport_collection& transports() = 0;
    virtual i_net_packet_builder::u_ptr_t create_packet_builder(transport_id_t transport_id) = 0;
};

}

#endif // MPL_I_NET_MODULE_H
