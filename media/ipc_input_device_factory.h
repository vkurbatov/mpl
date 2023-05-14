#ifndef MPL_IPC_INPUT_DEVICE_FACTORY_H
#define MPL_IPC_INPUT_DEVICE_FACTORY_H

#include "core/i_shared_data_manager.h"
#include "i_device_factory.h"

namespace mpl::media
{

class ipc_input_device_factory : public i_device_factory
{
    i_shared_data_manager&  m_shared_data_manager;
public:


    using u_ptr_t = std::unique_ptr<ipc_input_device_factory>;
    using s_ptr_t = std::shared_ptr<ipc_input_device_factory>;

    static u_ptr_t create(i_shared_data_manager& shared_data_manager);

    ipc_input_device_factory(i_shared_data_manager& shared_data_manager);

    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};

}

#endif // MPL_IPC_INPUT_DEVICE_FACTORY_H
