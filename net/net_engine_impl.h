#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"
#include "utils/task_manager_impl.h"

namespace mpl::net
{

class net_engine_impl : public i_net_engine
{
    task_manager_impl*      m_task_manager;
    // pt::io::io_core     m_io_core;


public:

    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using s_ptr_t = std::shared_ptr<net_engine_impl>;

    static net_engine_impl& get_instance();

    static u_ptr_t create(task_manager_impl* task_manager = nullptr);

    net_engine_impl(task_manager_impl* task_manager = nullptr);

    template<typename T>
    T& get();

    // i_net_engine interface
public:

    bool start() override;
    bool stop() override;
    bool is_started() const;

};

}

#endif // MPL_NET_ENGINE_IMPL_H
