#ifndef MPL_LOG_TOOLS_H
#define MPL_LOG_TOOLS_H

#include "i_logger.h"
#include <sstream>
#include <cstdint>

namespace mpl::log
{

void set_log_level(log_level_t level);
i_logger& get_logger();
bool has_log_level(log_level_t level);

void do_log(log_level_t level
            , const std::string_view& file
            , std::int32_t line
            , const std::string_view& msg);

void build_message(std::ostringstream& msg);

template<typename T, typename... Args>
void build_message(std::ostringstream& msg
                   , const T& value
                   , const Args&... args)
{
    msg << value;
    build_message(msg
                  , args...);
}

template<typename... Args>
void log_wrapper(log_level_t level
                 , const std::string_view& file
                 , std::int32_t line
                 , const Args&... args)
{
    std::ostringstream msg;
    build_message(msg
                  , args...);
    do_log(level
           , file
           , line
           , msg.str());
}


}

#define mpl_log(level, ...) \
if (mpl::log::has_log_level(level)) \
{\
    mpl::log::log_wrapper(level, __FILE__, __LINE__, __VA_ARGS__);\
}

#define mpl_log_trace(...) mpl_log(mpl::log::log_level_t::trace, __VA_ARGS__)
#define mpl_log_debuf(...) mpl_log(mpl::log::log_level_t::debug, __VA_ARGS__)
#define mpl_log_info(...) mpl_log(mpl::log::log_level_t::info, __VA_ARGS__)
#define mpl_log_warning(...) mpl_log(mpl::log::log_level_t::warning, __VA_ARGS__)
#define mpl_log_error(...) mpl_log(mpl::log::log_level_t::error, __VA_ARGS__)
#define mpl_log_fatal(...) mpl_log(mpl::log::log_level_t::fatal, __VA_ARGS__)

#endif // MPL_LOG_TOOLS_H
