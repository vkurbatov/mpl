#include "audio_format_impl.h"
#include "audio_info.h"

#include "media_utils.h"

#include "core/property_writer.h"
#include "core/option_helper.h"
#include "core/option_types.h"

namespace mpl::media
{

audio_format_impl::u_ptr_t audio_format_impl::create(const audio_format_id_t &format_id
                                                     , int32_t sample_rate
                                                     , int32_t channels)
{
    return std::make_unique<audio_format_impl>(format_id
                                               , sample_rate
                                               , channels);
}

audio_format_impl::u_ptr_t audio_format_impl::create(const i_audio_format &other)
{
    return std::make_unique<audio_format_impl>(other);
}

audio_format_impl::u_ptr_t audio_format_impl::create(const i_property &params)
{
    return std::make_unique<audio_format_impl>(params);
}

audio_format_impl::audio_format_impl(const audio_format_id_t &format_id
                                     , int32_t sample_rate
                                     , int32_t channels)
    : m_format_id(format_id)
    , m_sample_rate(sample_rate)
    , m_channels(channels)
{

}

audio_format_impl::audio_format_impl(const i_audio_format &other)
    : m_format_id(other.format_id())
    , m_sample_rate(other.sample_rate())
    , m_channels(other.channels())
{
    m_options.merge(other.options());
}

audio_format_impl::audio_format_impl(const i_property &params)
    : audio_format_impl()
{
    set_params(params);
}

audio_format_impl &audio_format_impl::set_format_id(const audio_format_id_t &format_id)
{
    m_format_id = format_id;
    return *this;
}

audio_format_impl &audio_format_impl::set_sample_rate(int32_t sample_rate)
{
    m_sample_rate = sample_rate;
    return *this;
}

audio_format_impl &audio_format_impl::set_channels(int32_t channels)
{
    m_channels = channels;
    return *this;
}

audio_format_impl &audio_format_impl::set_options(const i_option &options)
{
    m_options.assign(options);
    return *this;
}

audio_format_impl &audio_format_impl::set_options(option_impl &&options)
{
    m_options = std::move(options);
    return *this;
}

audio_format_impl &audio_format_impl::assign(const i_audio_format &other)
{
    m_format_id = other.format_id();
    m_sample_rate = other.sample_rate();
    m_channels = other.channels();
    m_options.assign(other.options());

    return *this;
}

bool audio_format_impl::set_params(const i_property &params)
{
    property_reader reader(params);
    if (reader.get("media_type", media_type_t::video) == media_type_t::video)
    {
        return reader.get("format", m_format_id)
                | reader.get("sample_rate", m_sample_rate)
                | reader.get("channels", m_channels)
                | utils::convert_format_options(params, m_options);
    }
    return false;
}

bool audio_format_impl::get_params(i_property &params) const
{
    property_writer writer(params);

    if (writer.set("media_type", media_type_t::video)
            && writer.set("format", m_format_id)
            && writer.set("sample_rate", m_sample_rate)
            && writer.set("channels", m_channels))
    {
        utils::convert_format_options(m_options, params);
        return true;
    }

    return false;
}

i_option& audio_format_impl::options()
{
    return m_options;
}

media_type_t audio_format_impl::media_type() const
{
    return media_type_t::audio;
}

bool audio_format_impl::is_encoded() const
{
    return audio_format_info_t::get_info(m_format_id).encoded;
}

bool audio_format_impl::is_convertable() const
{
    return audio_format_info_t::get_info(m_format_id).convertable;
}

i_media_format::u_ptr_t audio_format_impl::clone() const
{
    if (auto clone_format = create(m_format_id
                                   , m_sample_rate
                                   , m_channels))
    {
        clone_format->m_options = m_options;

        return clone_format;
    }

    return nullptr;
}

bool audio_format_impl::is_equal(const i_media_format &other) const
{
    if (is_compatible(other))
    {
        return m_options.is_equal(other.options());
    }

    return false;
}

bool audio_format_impl::is_compatible(const i_media_format &other) const
{
    if (other.media_type() == media_type())
    {
        const auto& audio_format = static_cast<const i_audio_format&>(other);
        return audio_format.format_id() == format_id()
                && audio_format.sample_rate() == sample_rate()
                && audio_format.channels() == channels();
    }

    return false;
}

const i_option& audio_format_impl::options() const
{
    return m_options;
}


bool audio_format_impl::is_valid() const
{
    return format_id() != audio_format_id_t::undefined
            && sample_rate() > 0
            && channels() > 0;
}

audio_format_id_t audio_format_impl::format_id() const
{
    return m_format_id;
}

int32_t audio_format_impl::sample_rate() const
{
    return m_sample_rate;
}

int32_t audio_format_impl::channels() const
{
    return m_channels;
}

}
