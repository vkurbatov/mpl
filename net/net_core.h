#ifndef MPL_NET_CORE_H
#define MPL_NET_CORE_H

#include "i_transport_factory.h"
#include "i_net_packet.h"
#include "socket/socket_types.h"

namespace mpl
{

class i_timer_manager;
class i_task_manager;

namespace net
{

struct net_core_config_t;
struct ice_config_t;
struct tls_config_t;

class net_core
{
    net_core();
    ~net_core();

public:
    static net_core& get_instance();

    bool init(i_timer_manager* task_manager = nullptr);
    bool cleanup();
    bool is_init() const;

    i_timer_manager& timer_manager();
    i_task_manager& task_manager();

    i_transport_factory& builtin_socket_factory();

    i_transport_factory::u_ptr_t create_udp_factory();
    i_transport_factory::u_ptr_t create_ice_factory(const ice_config_t& ice_config
                                                    , i_transport_factory* socket_factory = nullptr
                                                    , i_timer_manager* timer_manager = nullptr);
    i_transport_factory::u_ptr_t create_tls_factory(const tls_config_t& tls_config);

    i_data_object::u_ptr_t create_data(const void* data
                                       , std::size_t size);

    i_net_packet::u_ptr_t create_packet(transport_id_t transport_id
                                       , const void* data
                                       , std::size_t size
                                       , bool copy = false
                                       , const socket_address_t& address = {});

    i_net_packet::u_ptr_t create_packet(transport_id_t transport_id
                                        , i_data_object::s_ptr_t shared_data
                                        , const socket_address_t& address = {});

};

}

}

#endif // MPL_NET_CORE_H
