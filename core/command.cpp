#include "command.h"

#include <vector>
#include "tools/base/sync_base.h"
#include <mutex>

namespace mpl
{

namespace detail
{
    static base::spin_lock safe_mutex;
    static std::vector<std::string> commands;

    inline command_t::command_id_t register_command(const std::string_view &command_name)
    {
        std::lock_guard lock(safe_mutex);

        command_t::command_id_t command_id = commands.size();
        commands.emplace_back(command_name);

        return command_id;
    }
}

command_t::command_id_t command_t::register_command(const std::string_view &command_name)
{
    return detail::register_command(command_name);
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
