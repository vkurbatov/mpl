#include "audio_format_impl.h"
#include "audio_info.h"

namespace mpl
{

audio_format_impl::u_ptr_t audio_format_impl::create(const audio_format_id_t &format_id
                                                     , int32_t sample_rate
                                                     , int32_t channels)
{
    return std::make_unique<audio_format_impl>(format_id
                                               , sample_rate
                                               , channels);
}

audio_format_impl::audio_format_impl(const audio_format_id_t &format_id
                                     , int32_t sample_rate
                                     , int32_t channels)
    : m_format_id(format_id)
    , m_sample_rate(sample_rate)
    , m_channels(channels)
{

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

audio_format_impl &audio_format_impl::set_sample_channels(int32_t channels)
{
    m_channels = channels;
    return *this;
}


audio_format_impl &audio_format_impl::set_params(i_property::u_ptr_t &&params)
{
    m_params = std::move(params);
    return *this;
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
        if (m_params != nullptr)
        {
            clone_format->set_params(m_params->clone());
        }

        return clone_format;
    }

    return nullptr;
}

bool audio_format_impl::is_equal(const i_media_format &other) const
{
    if (is_compatible(other))
    {
        auto self_params = params();
        auto other_params = other.params();
        return (self_params == other_params)
                || (self_params != nullptr
                    && other_params != nullptr
                    && self_params->is_equal(*other_params));
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

const i_property *audio_format_impl::params() const
{
    return m_params.get();
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
