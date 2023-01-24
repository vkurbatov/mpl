#include "audio_format_helper.h"
#include "audio_info.h"
#include "i_audio_format.h"

namespace mpl::media
{

audio_format_helper::audio_format_helper(const i_audio_format &audio_format)
    : m_audio_format(audio_format)
{

}

std::size_t audio_format_helper::bits_per_sample() const
{
    return audio_format_info_t::get_info(m_audio_format.format_id()).bps;
}

std::size_t audio_format_helper::sample_size() const
{
    return (bits_per_sample() * m_audio_format.channels()) / 8;
}

timestamp_t audio_format_helper::duration_form_size(std::size_t size) const
{
    return m_audio_format.is_valid()
            ? (size * durations::seconds(1))
              / size_from_duration(durations::seconds(1))
            : 0;
}

timestamp_t audio_format_helper::duration_form_samples(uint32_t samples) const
{
    return duration_form_size(size_from_samples(samples));
}

std::size_t audio_format_helper::size_from_duration(timestamp_t duration) const
{
    return (duration * m_audio_format.sample_rate() * bits_per_sample() * m_audio_format.channels())
            / (8 * durations::seconds(1));
}

std::size_t audio_format_helper::size_from_samples(uint32_t samples) const
{
    return samples * sample_size();
}

uint32_t audio_format_helper::samples_from_size(uint32_t size) const
{
    return m_audio_format.is_valid()
            ? size / sample_size()
            : 0;
}

uint32_t audio_format_helper::samples_from_duration(timestamp_t duration) const
{
    return m_audio_format.is_valid()
            ? size_from_duration(duration) / sample_size()
            : 0;
}



}
