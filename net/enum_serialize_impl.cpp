#include "utils/enum_serialize_defs.h"

#include "net_types.h"

namespace mpl
{

using namespace net;

__declare_enum_serializer(transport_id_t)
__declare_enum_serializer(tcp_type_t)
__declare_enum_serializer(role_t)

}
