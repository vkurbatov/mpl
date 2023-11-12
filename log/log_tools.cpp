#include "log_tools.h"
#include "logger_impl.h"
#include "log_message.h"

#include "core/thread_info.h"
#include "utils/time_utils.h"


namespace mpl::log
{

struct single_logger_t
{
    logger_impl             logger;


    static single_logger_t& get_instance()
    {
        static single_logger_t single_logger;
        return single_logger;
    }
};

void do_log(log_level_t level
            , const std::string_view &file
            , int32_t line
            , const std::string_view& msg)
{
    log_message_t message(level
                          , utils::time::now()
                          , ""
                          , thread_info_t::current().name
                          , file
                          , line
                          , msg);


    single_logger_t::get_instance().logger.log(message);
}

bool has_log_level(log_level_t level)
{
    return level >= single_logger_t::get_instance().logger.level();
}

i_logger &get_logger()
{
    return single_logger_t::get_instance().logger;
}

void set_log_level(log_level_t level)
{
    single_logger_t::get_instance().logger.set_level(level);
}

void build_message(std::ostringstream &msg)
{
    // nothing
}


}
