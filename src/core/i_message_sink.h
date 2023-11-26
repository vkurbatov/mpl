#ifndef MPL_I_MESSAGE_SINK_H
#define MPL_I_MESSAGE_SINK_H

#include <memory>

namespace mpl
{

class i_message;

class i_message_sink
{
public:
    using u_ptr_t = std::unique_ptr<i_message_sink>;
    using s_ptr_t = std::shared_ptr<i_message_sink>;

    virtual ~i_message_sink() = default;
    virtual bool send_message(const i_message& message) = 0;
};

}

#endif // MPL_I_MESSAGE_SINK_H
