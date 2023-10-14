#ifndef MPL_NET_ENGINE_IMPL_H
#define MPL_NET_ENGINE_IMPL_H

#include "i_net_engine.h"
#include "tools/io/io_core.h"

namespace mpl::net
{

class net_engine_impl : public i_net_engine
{
    io::io_core     m_io_core;

public:

    using u_ptr_t = std::unique_ptr<net_engine_impl>;
    using s_ptr_t = std::shared_ptr<net_engine_impl>;

    static net_engine_impl& get_instance();

    static u_ptr_t create(std::size_t max_workers = 0);

    net_engine_impl(std::size_t max_workers = 0);

    // i_net_engine interface
public:

    bool start() override;
    bool stop() override;
    bool is_started() const;

    i_transport_factory::u_ptr_t create_factory(transport_id_t transport_id
                                                , const i_property *factory_params) override;
};

}

#endif // MPL_NET_ENGINE_IMPL_H