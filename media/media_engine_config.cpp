#include "media_engine_config.h"

namespace mpl::media
{


media_engine_config_t::media_engine_config_t(const std::string_view &ipc_name
                                             , std::size_t ipc_size)
    : ipc_name(ipc_name)
    , ipc_size(ipc_size)
{

}



}
