#ifndef MPL_LOG_MESSAGE_H
#define MPL_LOG_MESSAGE_H

#include "core/time_types.h"
#include "log_types.h"
#include <string>

namespace mpl::log
{

struct log_message_t
{
    log_level_t     level;
    timestamp_t     timestamp;
    std::string     thread_name;
    std::string     src;
    std::int32_t    line;
    std::string     message;

    log_message_t(log_level_t level = log_level_t::trace
                  , timestamp_t timestamp = timestamp_null
                  , const std::string_view& thread_name = {}
                  , const std::string_view& src = {}
                  , std::int32_t line = -1
                  , const std::string_view& message = {});

    std::string to_string() const;
};

}


#endif // MPL_LOG_MESSAGE_H
