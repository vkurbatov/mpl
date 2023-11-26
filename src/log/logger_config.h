#ifndef MPL_LOGGER_CONFIG_H
#define MPL_LOGGER_CONFIG_H

#include "log_types.h"

namespace mpl::log
{

struct logger_config_t
{
    log_level_t     level;
    logger_config_t(log_level_t level = log_level_t::trace);
};

}

#endif // MPL_LOGGER_CONFIG_H
