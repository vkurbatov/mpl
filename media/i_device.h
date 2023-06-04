#ifndef MPL_MEDIA_I_DEVICE_H
#define MPL_MEDIA_I_DEVICE_H

#include "device_types.h"
#include "core/i_message_channel.h"

namespace mpl::media
{

class i_device : public i_message_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_device>;
    using s_ptr_t = std::shared_ptr<i_device>;
    using w_ptr_t = std::weak_ptr<i_device>;

    virtual device_type_t device_type() const = 0;
};

}

#endif // MPL_MEDIA_I_DEVICE_H
