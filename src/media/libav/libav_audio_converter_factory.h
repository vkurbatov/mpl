#ifndef MPL_MEDIA_LIBAV_AUDIO_CONVERTER_FACTORY_H
#define MPL_MEDIA_LIBAV_AUDIO_CONVERTER_FACTORY_H

#include "media/i_media_converter_factory.h"

namespace mpl::media
{

class libav_audio_converter_factory : public i_media_converter_factory
{
public:

    static libav_audio_converter_factory& get_instance();

    libav_audio_converter_factory();

    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_property &params) override;
};

}

#endif // MPL_MEDIA_LIBAV_AUDIO_CONVERTER_FACTORY_H