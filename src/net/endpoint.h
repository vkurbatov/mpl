#ifndef MPL_NET_ENDPOINT_H
#define MPL_NET_ENDPOINT_H

#include "net_types.h"

namespace mpl::net
{

struct endpoint_t
{
    const transport_id_t transport_id;

    endpoint_t();

    endpoint_t(const endpoint_t& other) = default;
    endpoint_t(endpoint_t&& other) = default;
    endpoint_t& operator=(const endpoint_t& other);
    endpoint_t& operator=(endpoint_t&& other) = default;

    virtual bool operator == (const endpoint_t& other) const;
    virtual bool operator != (const endpoint_t& other) const;

    virtual bool is_valid() const;

protected:
    endpoint_t(transport_id_t transport_id);
};

}

#endif // MPL_NET_ENDPOINT_H
