#include "audio_info.h"
#include "audio_format_info.h"
#include "i_audio_format.h"

namespace mpl::media
{

audio_info_t::audio_info_t(audio_format_id_t format_id
                             , uint32_t sample_rate
                             , uint32_t channels)
    : format_id(format_id)
    , sample_rate(sample_rate)
    , channels(channels)
{

}

audio_info_t::audio_info_t(const i_audio_format &audio_format)
    : audio_info_t(audio_format.format_id()
                    , audio_format.sample_rate()
                    , audio_format.channels())
{

}

bool audio_info_t::operator ==(const audio_info_t &other) const
{
    return format_id == other.format_id
            && sample_rate == other.sample_rate
            && channels == other.channels;
}

bool audio_info_t::operator !=(const audio_info_t &other) const
{
    return ! operator == (other);
}

std::size_t audio_info_t::bps() const
{
    return audio_format_info_t::get_info(format_id).bps;
}

std::size_t audio_info_t::sample_size() const
{
    return (bps() * channels) / 8;
}

std::size_t audio_info_t::size_from_samples(std::size_t samples) const
{
    return samples * sample_size();
}

std::size_t audio_info_t::size_from_duration(timestamp_t duration) const
{
    return (duration * sample_rate * bps() * channels)
            / (8 * durations::seconds(1));
}

std::size_t audio_info_t::samples_from_size(std::size_t size) const
{
    if (auto ss = sample_size())
    {
        return size / ss;
    }

    return 0;
}

std::size_t audio_info_t::samples_from_duration(timestamp_t duration) const
{
    if (auto ss = sample_size())
    {
        return size_from_duration(duration) / ss;
    }

    return 0;
}

timestamp_t audio_info_t::duration_from_size(std::size_t size) const
{
    if (auto sd = size_from_duration(durations::seconds(1)))
    {
        return (size * durations::seconds(1))
                / sd;
    }

    return 0;
}

timestamp_t audio_info_t::duration_from_samples(std::size_t samples) const
{
    return duration_from_size(size_from_samples(samples));
}

bool audio_info_t::is_valid() const
{
    return audio_format_info_t::get_info(format_id).convertable
            && sample_rate > 0
            && channels > 0;
}

}
