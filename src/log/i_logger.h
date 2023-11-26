#ifndef MPL_LOG_I_LOGGER_H
#define MPL_LOG_I_LOGGER_H

#include "log_types.h"
#include <memory>
#include <functional>

namespace mpl::log
{

struct log_message_t;

class i_logger
{
public:
    class i_listener
    {
    public:
        using u_ptr_t = std::unique_ptr<i_listener>;
        using s_ptr_t = std::shared_ptr<i_listener>;

        virtual ~i_listener() = default;
        virtual bool on_log(const log_message_t& message) = 0;
    };

    using u_ptr_t = std::unique_ptr<i_logger>;
    using s_ptr_t = std::shared_ptr<i_logger>;

    virtual ~i_logger() = default;

    virtual log_level_t level() const = 0;
    virtual void set_listener(i_listener* listener) = 0;
    virtual bool log(const log_message_t& message) = 0;
};

}

#endif // MPL_LOG_I_LOGGER_H
