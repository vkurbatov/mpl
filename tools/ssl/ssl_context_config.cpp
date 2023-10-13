#include "ssl_context_config.h"
#include "ssl_utils.h"

namespace ssl
{

ssl_context_config_t::ssl_context_config_t(ssl_method_t method
                                           , ssl_options_flags_t options
                                           , ssl_session_cache_flags_t cache_flags
                                           , ssl_verify_flags_t verify_flags)
    : method(method)
    , options(options)
    , cache_flags(cache_flags)
    , verify_flags(verify_flags)
{

}

ssl_context_config_t &ssl_context_config_t::set_flag(const ssl_options_flags_t &option, bool enable)
{
    set_enum_flag(options, option, enable);
    return *this;
}

ssl_context_config_t &ssl_context_config_t::set_flag(const ssl_session_cache_flags_t &cache_flag, bool enable)
{
    set_enum_flag(cache_flags, cache_flag, enable);
    return *this;
}

ssl_context_config_t &ssl_context_config_t::set_flag(const ssl_verify_flags_t &verify_flag, bool enable)
{
    set_enum_flag(verify_flags, verify_flag, enable);
    return *this;
}

bool ssl_context_config_t::get_flag(const ssl_options_flags_t &option)
{
    return get_enum_flag(options, option);
}

bool ssl_context_config_t::get_flag(const ssl_session_cache_flags_t &cache_flag)
{
    return get_enum_flag(cache_flags, cache_flag);
}

bool ssl_context_config_t::get_flag(const ssl_verify_flags_t &verify_flag)
{
    return get_enum_flag(verify_flags, verify_flag);
}


}
