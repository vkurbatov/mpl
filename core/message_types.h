#ifndef MPL_MESSAGE_TYPES_H
#define MPL_MESSAGE_TYPES_H

#include <cstdint>

namespace mpl
{

enum class message_category_t
{
    undefined = 0,
    data,
    command,
    event,
    packet,
    application
};

using message_subclass_t = std::uint32_t;

constexpr message_subclass_t message_core_class = 0;

}

#endif // MPL_MESSAGE_TYPES_H
