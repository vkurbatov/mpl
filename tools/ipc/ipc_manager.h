#ifndef IPC_MANAGER_H
#define IPC_MANAGER_H

#include "ipc_types.h"
#include <functional>

namespace ipc
{

class ipc_segment;

class ipc_manager
{
public:
    struct config_t
    {
        std::string     name;
        std::size_t     size;

        config_t(const std::string_view& name
                 , std::size_t size = 0);

        bool is_valid() const;
        bool has_create() const;
    };

    using ipc_shmem_manager_ptr_t = std::shared_ptr<ipc_shmem_manager_t>;
private:
    config_t                    m_config;
    ipc_shmem_manager_ptr_t     m_native_manager;
public:

    static void remove_by_name(const std::string& name);
    static ipc_shmem_manager_ptr_t create_native_manager(const config_t& config);

    ipc_manager(const config_t& config);
    ~ipc_manager();

    ipc_segment create_segment(const std::string& name
                               , std::size_t size);

    /*template<typename IPCObject, class ...Args>
    IPCObject create_object(Args&& ...args);*/

    std::string name() const;

    void* pointer();

    std::size_t total_size() const;

    std::size_t free_size() const;

    ipc_shmem_manager_t& native_manager();

    bool is_valid() const;
    bool is_writeble() const;

};

}

#endif // IPC_MANAGER_H
