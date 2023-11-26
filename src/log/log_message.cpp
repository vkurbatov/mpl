#include "log_message.h"
#include "utils/time_utils.h"
#include "utils/enum_utils.h"

namespace mpl::log
{

log_message_t::log_message_t(log_level_t level
                             , timestamp_t timestamp
                             , const std::string_view &thread_name
                             , const std::string_view &src
                             , int32_t line
                             , const std::string_view &message)
    : level(level)
    , timestamp(timestamp)
    , thread_name(thread_name)
    , src(src)
    , line(line)
    , message(message)
{

}

std::string log_message_t::to_string() const
{
    std::string result;
    result.append(utils::time::to_string(timestamp))
            .append(" ").append(utils::enum_to_string(level))
            .append(" ").append(thread_name)
            .append(" ").append(src)
            .append("(").append(std::to_string(line)).append(")")
            .append(" ").append(message);

    return result;
}

}
