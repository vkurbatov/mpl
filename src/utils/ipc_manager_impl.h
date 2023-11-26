#ifndef MPL_UTILS_IPC_MANAGER_IMPL_H
#define MPL_UTILS_IPC_MANAGER_IMPL_H

#include "core/i_shared_data_manager.h"

namespace mpl
{

class ipc_manager_factory
{
public:
    static ipc_manager_factory& get_instance();
    i_shared_data_manager::u_ptr_t create_shared_data_manager(const std::string_view& name
                                                              , std::size_t total_size);
};

}

#endif // MPL_UTILS_IPC_MANAGER_IMPL_H
