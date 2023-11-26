#ifndef MPL_I_CHANNEL_H
#define MPL_I_CHANNEL_H

#include "channel_control.h"
#include <memory>

namespace mpl
{

class i_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_channel>;
    using s_ptr_t = std::shared_ptr<i_channel>;
    using w_ptr_t = std::weak_ptr<i_channel>;

    virtual ~i_channel() = default;
    virtual bool control(const channel_control_t& control) = 0;
    virtual bool is_open() const = 0;
    virtual channel_state_t state() const = 0;
};

}

#endif // MPL_I_CHANNEL_H
