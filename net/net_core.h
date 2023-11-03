#ifndef NET_CORE_H
#define NET_CORE_H

#include "net/i_net_engine.h"

namespace mpl::net
{

struct net_engine_config_t;

class net_core
{
public:
    static i_net_engine& get_engine(const net_engine_config_t& engine_config);

};

}

#endif // NET_CORE_H
