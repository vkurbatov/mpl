#ifndef APM_DEVICE_PARAMS_H
#define APM_DEVICE_PARAMS_H

#include "tools/wap/wap_processor.h"

namespace mpl::media
{

struct apm_device_params_t
{    
    using wap_config_t = pt::wap::wap_processor::config_t;

    wap_config_t    wap_config;

    apm_device_params_t(const wap_config_t& wap_config = {});

    bool is_valid() const;
};

}

#endif // APM_DEVICE_PARAMS_H
