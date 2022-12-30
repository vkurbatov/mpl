#include "video_format_impl.h"
#include "video_info.h"

namespace mpl
{

video_format_impl::u_ptr_t video_format_impl::create(const video_format_id_t &format_id
                                                     , int32_t width
                                                     , int32_t height
                                                     , double frame_rate)
{
    return std::make_unique<video_format_impl>(format_id
                                               , width
                                               , height
                                               , frame_rate);
}

video_format_impl::video_format_impl(const video_format_id_t &format_id
                                     , int32_t width
                                     , int32_t height
                                     , double frame_rate)
    : m_format_id(format_id)
    , m_width(width)
    , m_height(height)
    , m_frame_rate(frame_rate)
{

}

video_format_impl::video_format_impl(const i_video_format &other)
    : video_format_impl(other.format_id()
                        , other.width()
                        , other.height()
                        , other.frame_rate())
{
    m_options.merge(other.options());
}

video_format_impl &video_format_impl::set_format_id(const video_format_id_t &format_id)
{
    m_format_id = format_id;
    return *this;
}

video_format_impl &video_format_impl::set_width(int32_t width)
{
    m_width = width;
    return *this;
}

video_format_impl &video_format_impl::set_heigth(int32_t height)
{
    m_height = height;
    return *this;
}

video_format_impl &video_format_impl::set_frame_rate(double frame_rate)
{
    m_frame_rate = m_frame_rate;
    return *this;
}

video_format_impl &video_format_impl::set_options(option_impl &&options)
{
    m_options = std::move(options);
    return *this;
}

video_format_impl &video_format_impl::set_options(const i_option &options)
{
    m_options.assign(options);
    return *this;
}

video_format_impl &video_format_impl::assign(const i_video_format &other)
{
    m_format_id = other.format_id();
    m_width = other.width();
    m_height = other.height();
    m_frame_rate = other.frame_rate();
    m_options.assign(other.options());

    return *this;
}

i_option &video_format_impl::options()
{
    return m_options;
}

media_type_t video_format_impl::media_type() const
{
    return media_type_t::video;
}

bool video_format_impl::is_encoded() const
{
    return video_format_info_t::get_info(m_format_id).encoded;
}

bool video_format_impl::is_convertable() const
{
    return video_format_info_t::get_info(m_format_id).convertable;
}

i_media_format::u_ptr_t video_format_impl::clone() const
{
    if (auto clone_format = create(m_format_id
                                   , m_width
                                   , m_height
                                   , m_frame_rate))
    {
        clone_format->m_options = m_options;

        return clone_format;
    }

    return nullptr;
}

bool video_format_impl::is_equal(const i_media_format &other) const
{
    if (is_compatible(other))
    {
        const auto& video_format = static_cast<const i_video_format&>(other);
        return frame_rate() == video_format.frame_rate()
                && options().is_equal(other.options());
    }

    return false;
}

bool video_format_impl::is_compatible(const i_media_format &other) const
{
    if (other.media_type() == media_type())
    {
        const auto& video_format = static_cast<const i_video_format&>(other);
        return video_format.format_id() == format_id()
                && video_format.width() == width()
                && video_format.height() == height();
    }

    return false;
}

const i_option &video_format_impl::options() const
{
    return m_options;
}

bool video_format_impl::is_valid() const
{
    return format_id() != video_format_id_t::undefined
            && width() > 0
            && height() > 0;
}

video_format_id_t video_format_impl::format_id() const
{
    return m_format_id;
}

int32_t video_format_impl::width() const
{
    return m_width;
}

int32_t video_format_impl::height() const
{
    return m_height;
}

double video_format_impl::frame_rate() const
{
    return m_frame_rate;
}

}
