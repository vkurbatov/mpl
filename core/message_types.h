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

using message_subtype_t = std::uint32_t;

constexpr message_subtype_t message_subtype_core_base = 0;
constexpr message_subtype_t message_subtype_range = 1000;

}

#endif // MPL_MESSAGE_TYPES_H
