#ifndef MPL_I_MESSAGE_TRANSCEIVER_H
#define MPL_I_MESSAGE_TRANSCEIVER_H

#include <memory>

namespace mpl
{

class i_message_sink;
class i_message_source;

class i_message_transceiver
{
public:
    using u_ptr_t = std::unique_ptr<i_message_transceiver>;
    using s_ptr_t = std::shared_ptr<i_message_transceiver>;

    virtual ~i_message_transceiver() = default;

    virtual i_message_sink* sink(std::size_t index) = 0;
    virtual i_message_source* source(std::size_t index) = 0;
};

}

#endif // MPL_I_MESSAGE_TRANSCEIVER_H
