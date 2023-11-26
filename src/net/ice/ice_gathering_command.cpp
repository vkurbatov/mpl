#include "ice_gathering_command.h"

namespace mpl::net
{

ice_gathering_command_t::ice_gathering_command_t(const ice_server_params_t::array_t &ice_servers)
    : command_t(id
                , command_name)
{

}


}
