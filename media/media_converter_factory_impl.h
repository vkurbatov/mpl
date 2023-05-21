#ifndef MPL_MEDIA_CONVERTER_FACTORY_IMPL_H
#define MPL_MEDIA_CONVERTER_FACTORY_IMPL_H

#include "i_media_converter_factory.h"

namespace mpl::media
{

class media_converter_factory_impl : public i_media_converter_factory
{
    i_media_converter_factory&  m_audio_converter_factory;
    i_media_converter_factory&  m_video_converter_factory;
public:

    static media_converter_factory_impl& builtin_converter_factory();

    media_converter_factory_impl(i_media_converter_factory& audio_converter_factory
                                 , i_media_converter_factory& video_converter_factory);
    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_property& params) override;

};

}

#endif // MPL_MEDIA_CONVERTER_FACTORY_IMPL_H
