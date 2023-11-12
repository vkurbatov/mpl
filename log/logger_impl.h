#ifndef MPL_LOGGER_IMPL_H
#define MPL_LOGGER_IMPL_H

#include "logger_config.h"
#include "i_logger.h"

namespace mpl::log
{

class logger_impl : public i_logger
{
    logger_config_t     m_config;
    i_listener*         m_listener;
public:

    logger_impl(const logger_config_t& config = {}
                , i_listener* listener = nullptr);

    void set_level(log_level_t level);

    // i_logger interface
public:
    log_level_t level() const override;
    void set_listener(i_listener *listener) override;
    bool log(const log_message_t &message) override;
};

}

#endif // MPL_LOGGER_IMPL_H
