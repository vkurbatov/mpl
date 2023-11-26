#ifndef MPL_I_SYNC_SHARED_DATA_H
#define MPL_I_SYNC_SHARED_DATA_H

#include "i_shared_data.h"
#include "time_types.h"
#include <string>

namespace mpl
{

class i_sync_shared_data : public i_shared_data
{
public:
    using u_ptr_t = std::unique_ptr<i_sync_shared_data>;
    using s_ptr_t = std::shared_ptr<i_sync_shared_data>;
    using w_ptr_t = std::weak_ptr<i_sync_shared_data>;

    virtual ~i_sync_shared_data() = default;
    virtual std::string name() const = 0;
    virtual void notify() = 0;
    virtual bool wait(timestamp_t timeout) = 0;
};

}

#endif // MPL_I_SYNC_SHARED_DATA_H
