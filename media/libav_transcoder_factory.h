#ifndef MPL_LIBAV_TRANSCODER_FACTORY_H
#define MPL_LIBAV_TRANSCODER_FACTORY_H

#include "i_media_converter_factory.h"

namespace mpl::media
{

class libav_transcoder_factory : public i_media_converter_factory
{
    bool        m_encoder_factory;
public:

    libav_transcoder_factory(bool encoder_factory = true);

    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_media_format &media_format) override;

};

}

#endif // MPL_LIBAV_TRANSCODER_FACTORY_H