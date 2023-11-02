#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"
#include "tools/io/i_io_worker_factory.h"

namespace pt::io
{

class io_core;

}

namespace mpl
{

class i_task_manager;

namespace net
{

class net_engine_impl : public i_net_engine
{
public:
    struct config_t
    {
        std::size_t     max_workers = 1;
    };

private:

    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t         m_pimpl;

public:


    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using s_ptr_t = std::shared_ptr<net_engine_impl>;

    static net_engine_impl& get_instance();

    static u_ptr_t create(const config_t& config
                          , i_task_manager& task_manager);

    net_engine_impl(const config_t& config
                    , i_task_manager& task_manager);
    ~net_engine_impl();

    pt::io::io_core& io_core();

    // i_net_engine interface
public:

    bool start() override;
    bool stop() override;
    bool is_started() const;

};

}

}

#endif // MPL_NET_ENGINE_IMPL_H
