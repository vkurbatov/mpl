#ifndef MPL_NET_I_TRANSPORT_COLLECTION_H
#define MPL_NET_I_TRANSPORT_COLLECTION_H

#include "i_transport_factory.h"

namespace mpl::net
{

class i_transport_collection
{
public:
    using u_ptr_t = std::unique_ptr<i_transport_collection>;
    using s_ptr_t = std::shared_ptr<i_transport_collection>;

    virtual ~i_transport_collection() = default;

    virtual i_transport_factory* get_factory(transport_id_t transport_id) = 0;
};

}

#endif // MPL_NET_I_TRANSPORT_COLLECTION_H
