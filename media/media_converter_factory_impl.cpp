#include "media_converter_factory_impl.h"

namespace mpl::media
{


media_converter_factory_impl::media_converter_factory_impl(i_media_converter_factory &audio_converter_factory
                                                           , i_media_converter_factory &video_converter_factory)
    : m_audio_converter_factory(audio_converter_factory)
    , m_video_converter_factory(video_converter_factory)
{

}

i_media_converter::u_ptr_t media_converter_factory_impl::create_converter(const i_media_format &output_format)
{
    switch(output_format.media_type())
    {
        case media_type_t::audio:
            return m_audio_converter_factory.create_converter(output_format);
        break;
        case media_type_t::video:
            return m_video_converter_factory.create_converter(output_format);
        break;
        default:;
    }

    return nullptr;
}



}
