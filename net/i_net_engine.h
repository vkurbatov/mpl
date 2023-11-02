#ifndef MPL_I_NET_ENGINE_H
#define MPL_I_NET_ENGINE_H

#include "core/i_engine.h"

namespace mpl::net
{

class i_net_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_net_engine>;
    using s_ptr_t = std::shared_ptr<i_net_engine>;

    // ???
};

}


#endif // MPL_I_NET_ENGINE_H
