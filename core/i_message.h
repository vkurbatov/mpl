#ifndef MPL_I_MESSAGE_H
#define MPL_I_MESSAGE_H

#include "message_types.h"
#include <memory>
#include <cstdint>

namespace mpl
{

class i_message
{
public:
    using u_ptr_t = std::unique_ptr<i_message>;
    using s_ptr_t = std::shared_ptr<i_message>;
    using w_ptr_t = std::weak_ptr<i_message>;
    using r_ptr_t = i_message*;
    virtual ~i_message() = default;

    virtual message_category_t category() const = 0;
    virtual message_subclass_t subclass() const = 0;
    virtual u_ptr_t clone() const = 0;
};

}

#endif // MPL_I_MESSAGE_H
