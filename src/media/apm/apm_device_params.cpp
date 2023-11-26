#include "apm_device_params.h"

namespace mpl::media
{

apm_device_params_t::apm_device_params_t(const wap_config_t &wap_config)
    : wap_config(wap_config)
{

}

bool apm_device_params_t::is_valid() const
{
    return wap_config.is_valid();
}

}
