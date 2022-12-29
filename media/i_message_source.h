#ifndef MPL_I_MESSAGE_RECEIVER_H
#define MPL_I_MESSAGE_RECEIVER_H

#include <memory>

namespace mpl
{

class i_message_sink;

class i_message_source
{
public:
    using u_ptr_t = std::unique_ptr<i_message_source>;
    using s_ptr_t = std::shared_ptr<i_message_source>;

    virtual ~i_message_source() = default;

    virtual bool add_sink(i_message_sink* sink) = 0;
    virtual bool remove_sink(i_message_sink* sink) = 0;
};

}

#endif // MPL_I_MESSAGE_RECEIVER_H
