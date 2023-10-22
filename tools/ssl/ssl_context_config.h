#ifndef SSL_CONTEXT_CONFIG_H
#define SSL_CONTEXT_CONFIG_H

#include "ssl_types.h"

namespace pt::ssl
{

struct ssl_context_config_t
{
    ssl_method_t                method;
    ssl_options_flags_t         options;
    ssl_session_cache_flags_t   cache_flags;
    ssl_verify_flags_t          verify_flags;

    ssl_context_config_t(ssl_method_t method = ssl_method_t::default_method
                         , ssl_options_flags_t options = ssl_options_flags_t::none
                         , ssl_session_cache_flags_t cache_flags = ssl_session_cache_flags_t::none
                         , ssl_verify_flags_t verify_flags = ssl_verify_flags_t::none);

    ssl_context_config_t& set_flag(const ssl_options_flags_t& option, bool enable);
    ssl_context_config_t& set_flag(const ssl_session_cache_flags_t& cache_flag, bool enable);
    ssl_context_config_t& set_flag(const ssl_verify_flags_t& verify_flag, bool enable);

    bool get_flag(const ssl_options_flags_t& option);
    bool get_flag(const ssl_session_cache_flags_t& cache_flag);
    bool get_flag(const ssl_verify_flags_t& verify_flag);

};

}

#endif // SSL_CONTEXT_CONFIG_H
