#include "logger_impl.h"
#include "log_message.h"

#include <iostream>

namespace mpl::log
{

logger_impl::logger_impl(const logger_config_t &config
                         , i_listener *listener)
    : m_config(config)
    , m_listener(listener)
{

}

void logger_impl::set_level(log_level_t level)
{
    m_config.level = level;
}

log_level_t logger_impl::level() const
{
    return m_config.level;
}

void logger_impl::set_listener(i_listener *listener)
{
    m_listener = listener;
}

bool logger_impl::log(const log_message_t &message)
{
    if (auto listener = m_listener)
    {
        return listener->on_log(message);
    }
    else
    {
        std::clog << message.to_string() << std::endl;
    }

    return false;
}


}
