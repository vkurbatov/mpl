#ifndef MPL_NET_I_TLS_TRANSPORT_H
#define MPL_NET_I_TLS_TRANSPORT_H

#include "net/i_transport_channel.h"
#include "tls_types.h"
#include "tls_endpoint.h"

namespace mpl::net
{

class i_tls_transport : public i_transport_channel
{
public:
    using u_ptr_t = std::unique_ptr<i_tls_transport>;
    using s_ptr_t = std::shared_ptr<i_tls_transport>;
    using w_ptr_t = std::weak_ptr<i_tls_transport>;

    virtual role_t role() const = 0;
    virtual tls_method_t method() const = 0;

    virtual tls_endpoint_t get_local_endpoint() const = 0;
    virtual tls_endpoint_t get_remote_endpoint() const = 0;

    virtual bool set_local_endpoint(const tls_endpoint_t& endpoint) = 0;
    virtual bool set_remote_endpoint(const tls_endpoint_t& endpoint) = 0;
};

}

#endif // MPL_NET_I_TLS_TRANSPORT_H
