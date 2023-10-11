#ifndef MPL_NET_I_NET_ENGINE_H
#define MPL_NET_I_NET_ENGINE_H

#include "i_transport_factory.h"
#include <memory>

namespace mpl::net
{

class i_net_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_net_engine>;
    using s_ptr_t = std::shared_ptr<i_net_engine>;

    virtual ~i_net_engine() = default;
    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;
    virtual i_transport_factory::u_ptr_t create_factory(transport_id_t transport_id
                                                        , const i_property* factory_params) = 0;

};

}

#endif // MPL_NET_I_NET_ENGINE_H
