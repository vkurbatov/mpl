#ifndef MPL_NET_TLS_ENDPOINT_H
#define MPL_NET_TLS_ENDPOINT_H

#include "net/endpoint.h"
#include "tls_fingerprint.h"

namespace mpl::net
{

struct tls_endpoint_t : public endpoint_t
{
    tls_fingerprint_t           fingerprint;

    tls_endpoint_t(const tls_fingerprint_t& fingerprint = {});

    bool operator == (const tls_endpoint_t& other) const;

    // endpoint_t interface
public:
    bool operator ==(const endpoint_t &other) const override;
    bool is_valid() const override;
};

}

#endif // MPL_NET_TLS_ENDPOINT_H
