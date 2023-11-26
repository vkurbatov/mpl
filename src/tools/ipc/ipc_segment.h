#ifndef IPC_SEGMENT_H
#define IPC_SEGMENT_H

#include "ipc_types.h"

namespace pt::ipc
{

class ipc_segment
{
    struct segment_ctx_t;
    using ipc_shmem_manager_ptr_t = std::shared_ptr<ipc_shmem_manager_t>;

    ipc_mutex_t                 m_sync_mutex;

    ipc_shmem_manager_ptr_t     m_manager;
    segment_ctx_t*              m_segment_ctx;
    std::int32_t                m_trigger;


public:
    using u_ptr_t = std::unique_ptr<ipc_segment>;
    ipc_segment(const ipc_shmem_manager_ptr_t& manager
                , const std::string& name
                , std::size_t size);

    ~ipc_segment();

    void* try_map();
    void* map(timestamp_t timeout_ns);
    void* map();
    void unmap();

    std::string name() const;

    void notify(bool all);
    void wait();
    bool wait(timestamp_t timeout_ns);

    std::size_t size() const;

    bool is_valid() const;

};

}

#endif // IPC_SEGMENT_H
