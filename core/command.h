#ifndef MPL_COMMAND_H
#define MPL_COMMAND_H

#include "command_types.h"
#include <string>

namespace mpl
{

struct command_t
{
    const command_id_t  command_id;
    const std::string   name;

    virtual bool operator == (const command_t& other) const;
    virtual bool operator != (const command_t& other) const;

protected:
    command_t(const command_id_t command_id
              , const std::string_view& name = {});

    command_t(const command_t& other) = default;
    command_t(command_t&& other) = default;

    command_t& operator=(const command_t& other);
    command_t& operator=(command_t&& other);
};

}

#endif // MPL_COMMAND_H
