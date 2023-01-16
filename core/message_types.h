#ifndef MPL_MESSAGE_TYPES_H
#define MPL_MESSAGE_TYPES_H

namespace mpl
{

enum class message_category_t
{
    undefined = -1,
    frame,
    control,
    event
};

}

#endif // MPL_MESSAGE_TYPES_H
