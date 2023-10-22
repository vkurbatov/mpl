#ifndef IO_NET_UDP_LINK_H
#define IO_NETUDP_LINK_H

#include "tools/io/io_link_base.h"

namespace pt::io
{

struct udp_link_config_t;
struct ip_endpoint_t;

class udp_link : public io_link
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t         m_pimpl;
    friend struct pimpl_t;

public:

    using u_ptr_t = std::unique_ptr<udp_link>;
    static u_ptr_t create(io_core& core
                          , const udp_link_config_t& config);

    udp_link(io_core& core
             , const udp_link_config_t& config);
    ~udp_link();

    bool set_config(const udp_link_config_t& config);

    const udp_link_config_t& config();
    const ip_endpoint_t& local_endpoint() const;
    const ip_endpoint_t& remote_endpoint() const;

    bool set_local_endpoint(const ip_endpoint_t& endpoint);
    bool set_remote_endpoint(const ip_endpoint_t& endpoint);

    // i_io_link interface
public:
    link_type_t type() const override;
    bool control(link_control_id_t control_id) override;
    bool send_to(const message_t &message
                 , const endpoint_t &endpoint) override;
    bool get_endpoint(endpoint_t &endpoint) const override;
    bool set_endpoint(const endpoint_t &endpoint) override;
    bool is_open() const override;

};


}

#endif // IO_NETUDP_LINK_H
