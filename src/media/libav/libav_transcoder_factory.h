#ifndef MPL_MEDIA_LIBAV_TRANSCODER_FACTORY_H
#define MPL_MEDIA_LIBAV_TRANSCODER_FACTORY_H

#include "media/i_media_converter_factory.h"

namespace mpl::media
{

class libav_transcoder_factory : public i_media_converter_factory
{
    bool        m_encoder_factory;
public:

    libav_transcoder_factory(bool encoder_factory = true);

    static libav_transcoder_factory& encoder_factory();
    static libav_transcoder_factory& decoder_factory();

    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_property& params) override;

};

}

#endif // MPL_MEDIA_LIBAV_TRANSCODER_FACTORY_H
