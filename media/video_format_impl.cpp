#include "video_format_impl.h"
#include "video_format_info.h"

#include "media_utils.h"

#include "utils/property_writer.h"
#include "utils/option_helper.h"
#include "core/option_types.h"

namespace mpl::media
{

video_format_impl &video_format_impl::undefined_video_fromat()
{
    static video_format_impl undefined_format;
    return undefined_format;
}

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

video_format_impl::u_ptr_t video_format_impl::create(const video_info_t &video_info)
{
    return std::make_unique<video_format_impl>(video_info);
}

video_format_impl::u_ptr_t video_format_impl::create(const i_video_format &other)
{
    return std::make_unique<video_format_impl>(other);
}

video_format_impl::u_ptr_t video_format_impl::create(const i_property &params)
{
    return std::make_unique<video_format_impl>(params);
}

video_format_impl::video_format_impl(const video_format_id_t &format_id
                                     , int32_t width
                                     , int32_t height
                                     , double frame_rate)
    : m_video_info(format_id
                   , { width, height}
                   , frame_rate)
{

}

video_format_impl::video_format_impl(const video_info_t &video_info)
    : m_video_info(video_info)
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

video_format_impl::video_format_impl(const i_property &params)
    : video_format_impl()
{
    set_params(params);
}

const video_info_t &video_format_impl::video_info() const
{
    return m_video_info;
}

video_format_impl &video_format_impl::set_format_id(const video_format_id_t &format_id)
{
    m_video_info.format_id = format_id;
    return *this;
}

video_format_impl &video_format_impl::set_width(int32_t width)
{
    m_video_info.size.width = width;
    return *this;
}

video_format_impl &video_format_impl::set_height(int32_t height)
{
    m_video_info.size.height = height;
    return *this;
}

video_format_impl &video_format_impl::set_frame_rate(double frame_rate)
{
    m_video_info.frame_rate = frame_rate;
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

video_format_impl &video_format_impl::set_video_info(const video_info_t &info)
{
    m_video_info = info;
    return *this;
}

video_format_impl &video_format_impl::assign(const i_video_format &other)
{
    m_video_info.format_id = other.format_id();
    m_video_info.size.width = other.width();
    m_video_info.size.height = other.height();
    m_video_info.frame_rate = other.frame_rate();
    m_options.assign(other.options());

    return *this;
}

bool video_format_impl::set_params(const i_property &params)
{
    property_reader reader(params);
    if (reader.get("media_type", media_type_t::video) == media_type_t::video)
    {
        return reader.get("format", m_video_info.format_id)
                | reader.get("width", m_video_info.size.width)
                | reader.get("height", m_video_info.size.height)
                | reader.get("frame_rate", m_video_info.frame_rate)
                | utils::convert_format_options(params, m_options);
    }
    return false;
}

bool video_format_impl::get_params(i_property &params) const
{
    property_writer writer(params);

    if (writer.set("media_type", media_type_t::video)
            && writer.set("format", m_video_info.format_id)
            && writer.set("width", m_video_info.size.width)
            && writer.set("height", m_video_info.size.height)
            && writer.set("frame_rate", m_video_info.frame_rate))
    {
        utils::convert_format_options(m_options, params);
        return true;
    }

    return false;
}

i_property::u_ptr_t video_format_impl::get_params(const std::string &path) const
{
    if (auto params = property_helper::create_object())
    {
        if (property_writer(*params).set(path, *this))
        {
            return params;
        }
    }

    return nullptr;
}

option_impl &video_format_impl::options()
{
    return m_options;
}

media_type_t video_format_impl::media_type() const
{
    return media_type_t::video;
}

bool video_format_impl::is_encoded() const
{
    return video_format_info_t::get_info(m_video_info.format_id).encoded;
}

bool video_format_impl::is_convertable() const
{
    return video_format_info_t::get_info(m_video_info.format_id).convertable;
}

i_media_format::u_ptr_t video_format_impl::clone() const
{
    if (auto clone_format = create(m_video_info.format_id
                                   , m_video_info.size.width
                                   , m_video_info.size.height
                                   , m_video_info.frame_rate))
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
    return m_video_info.format_id;
}

int32_t video_format_impl::width() const
{
    return m_video_info.size.width;
}

int32_t video_format_impl::height() const
{
    return m_video_info.size.height;
}

double video_format_impl::frame_rate() const
{
    return m_video_info.frame_rate;
}

}
