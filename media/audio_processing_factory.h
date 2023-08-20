#ifndef MPL_AUDIO_PROCESSING_FACTORY_H
#define MPL_AUDIO_PROCESSING_FACTORY_H

#include "i_device_factory.h"

namespace mpl::media
{

class audio_processing_factory : public i_device_factory
{
public:
    using u_ptr_t = std::unique_ptr<audio_processing_factory>;
    using s_ptr_t = std::shared_ptr<audio_processing_factory>;

    static u_ptr_t create();

    audio_processing_factory();


    // i_device_factory interface
public:
    i_device::u_ptr_t create_device(const i_property &device_params) override;
};

}

#endif // MPL_AUDIO_PROCESSING_FACTORY_H
