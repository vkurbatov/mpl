#ifndef MPL_MESSAGE_TYPES_H
#define MPL_MESSAGE_TYPES_H

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

}

#endif // MPL_MESSAGE_TYPES_H
