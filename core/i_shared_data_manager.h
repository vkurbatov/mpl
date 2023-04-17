#ifndef MPL_I_SHARED_DATA_MANAGER_H
#define MPL_I_SHARED_DATA_MANAGER_H

#include "i_sync_shared_data.h"

namespace mpl
{

class i_shared_data_manager
{
public:
    using u_ptr_t = std::unique_ptr<i_shared_data_manager>;
    using s_ptr_t = std::shared_ptr<i_shared_data_manager>;

    virtual ~i_shared_data_manager() = default;
    virtual i_sync_shared_data::s_ptr_t query_data(const std::string_view& name
                                                    , std::size_t size) = 0;

    virtual std::size_t available_size() = 0;
};

}

#endif // MPL_I_SHARED_DATA_MANAGER_H
