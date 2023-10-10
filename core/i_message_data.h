#ifndef MPL_I_MESSAGE_DATA_H
#define MPL_I_MESSAGE_DATA_H

#include "i_message.h"

#include <cstdint>

namespace mpl
{

class i_option;
class i_buffer_collection;

class i_message_data: public i_message
{
public:

    using u_ptr_t = std::unique_ptr<i_message_data>;
    using s_ptr_t = std::shared_ptr<i_message_data>;
    virtual const i_buffer_collection& data() const = 0;
    virtual const i_option* options() const = 0;

};

}

#endif // MPL_I_MESSAGE_DATA_H
