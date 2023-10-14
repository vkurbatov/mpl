#include "command.h"

#include <vector>
#include "tools/base/sync_base.h"
#include <mutex>

namespace mpl
{

bool command_t::operator ==(const command_t &other) const
{
    return command_id == other.command_id
            && name == other.name;
}

bool command_t::operator !=(const command_t &other) const
{
        return ! operator == (other);
}

command_t::command_t(const command_id_t command_id
                     , const std::string_view &name)
    : command_id(command_id)
    , name(name)
{

}

command_t &command_t::operator=(const command_t &other)
{
    return *this;
}

command_t &command_t::operator=(command_t &&other)
{
    return *this;
}

}
