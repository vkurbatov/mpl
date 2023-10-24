#ifndef MPL_I_NET_ENGINE_H
#define MPL_I_NET_ENGINE_H

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

};

}


#endif // MPL_I_NET_ENGINE_H
