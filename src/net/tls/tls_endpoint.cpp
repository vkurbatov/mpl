#include "tls_endpoint.h"

namespace mpl::net
{

tls_endpoint_t::tls_endpoint_t(const tls_fingerprint_t &fingerprint)
    : endpoint_t(transport_id_t::tls)
    , fingerprint(fingerprint)
{

}

bool tls_endpoint_t::operator ==(const tls_endpoint_t &other) const
{
    return fingerprint == other.fingerprint;
}

bool tls_endpoint_t::operator ==(const endpoint_t &other) const
{
    return other.transport_id == transport_id_t::ice
            && *this == static_cast<const tls_endpoint_t&>(other);
}

bool tls_endpoint_t::is_valid() const
{
    return (transport_id == transport_id_t::tls);
}

}
