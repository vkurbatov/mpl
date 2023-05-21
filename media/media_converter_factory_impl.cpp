#include "media_converter_factory_impl.h"
#include "libav_audio_converter_factory.h"
#include "libav_video_converter_factory.h"
#include "core/property_reader.h"

namespace mpl::media
{


media_converter_factory_impl &media_converter_factory_impl::builtin_converter_factory()
{
    static media_converter_factory_impl single_media_converter_factory(libav_audio_converter_factory::get_instance()
                                                                       , libav_video_converter_factory::get_instance());
    return single_media_converter_factory;
}

media_converter_factory_impl::media_converter_factory_impl(i_media_converter_factory &audio_converter_factory
                                                           , i_media_converter_factory &video_converter_factory)
    : m_audio_converter_factory(audio_converter_factory)
    , m_video_converter_factory(video_converter_factory)
{

}

i_media_converter::u_ptr_t media_converter_factory_impl::create_converter(const i_property& params)
{
    property_reader reader(params);
    switch(reader.get<media_type_t>("format.media_type", media_type_t::undefined))
    {
        case media_type_t::audio:
            return m_audio_converter_factory.create_converter(params);
        break;
        case media_type_t::video:
            return m_video_converter_factory.create_converter(params);
        break;
        default:;
    }

    return nullptr;
}



}
