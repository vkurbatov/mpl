#ifndef MPL_NET_I_TRANSPORT_FACTORY_H
#define MPL_NET_I_TRANSPORT_FACTORY_H

#include "core/i_property.h"
#include "i_transport_channel.h"

namespace mpl::net
{

class i_transport_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_transport_factory>;

    virtual ~i_transport_factory() = default;
    virtual i_transport_channel::u_ptr_t create_transport(const i_property& params) = 0;

};

}

#endif // MPL_NET_I_TRANSPORT_FACTORY_H
