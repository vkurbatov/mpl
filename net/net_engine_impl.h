#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"

namespace mpl
{

class task_manager_impl;

namespace net
{

class net_engine_impl : public i_net_engine
{
    task_manager_impl&      m_task_manager;
public:

    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using s_ptr_t = std::shared_ptr<net_engine_impl>;

    static net_engine_impl& get_instance();

    static u_ptr_t create(task_manager_impl& task_manager);

    net_engine_impl(task_manager_impl& task_manager);

    template<typename T>
    T& get();

    // i_net_engine interface
public:

    bool start() override;
    bool stop() override;
    bool is_started() const;

};

}

}

#endif // MPL_NET_ENGINE_IMPL_H
