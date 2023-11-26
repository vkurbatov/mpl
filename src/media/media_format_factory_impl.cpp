#include "media_format_factory_impl.h"
#include "audio_format_impl.h"
#include "video_format_impl.h"

#include "utils/property_reader.h"

namespace mpl::media
{

media_format_factory_impl::u_ptr_t media_format_factory_impl::create(media_type_t media_type)
{
    return std::make_unique<media_format_factory_impl>(media_type);
}

media_format_factory_impl::media_format_factory_impl(media_type_t media_type)
    : m_media_type(media_type)
{

}

void media_format_factory_impl::set_media_type(media_type_t media_type)
{
    m_media_type = media_type;
}

media_type_t media_format_factory_impl::media_type() const
{
    return m_media_type;
}

i_media_format::u_ptr_t media_format_factory_impl::create_format(const i_property &format_params)
{
    switch(property_reader(format_params).get("media_type"
                                              , media_type()))
    {
        case media_type_t::audio:
            return audio_format_impl::create(format_params);
        break;
        case media_type_t::video:
            return video_format_impl::create(format_params);
        break;
        default:;
    }

    return nullptr;
}



}
