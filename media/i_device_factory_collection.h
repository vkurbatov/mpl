#ifndef MPL_MEDIA_I_DEVICE_COLLECTION_H
#define MPL_MEDIA_I_DEVICE_COLLECTION_H

#include "i_device_factory.h"
#include "device_types.h"

namespace mpl::media
{

class i_device_collection
{
public:
    using u_ptr_t = std::unique_ptr<i_device_collection>;
    using s_ptr_t = std::shared_ptr<i_device_collection>;

    virtual i_device_factory* get_factory(device_type_t device_type) = 0;
};

}

#endif // MPL_MEDIA_I_DEVICE_COLLECTION_H
