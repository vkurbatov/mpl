#include "audio_sample.h"
#include "audio_info.h"
#include "i_audio_format.h"

#include <cstring>

namespace mpl::media
{

sample_info_t::sample_info_t(audio_format_id_t format_id
                             , uint32_t sample_rate
                             , uint32_t channels)
    : format_id(format_id)
    , sample_rate(sample_rate)
    , channels(channels)
{

}

sample_info_t::sample_info_t(const i_audio_format &audio_format)
    : sample_info_t(audio_format.format_id()
                    , audio_format.sample_rate()
                    , audio_format.channels())
{

}

bool sample_info_t::operator ==(const sample_info_t &other) const
{
    return format_id == other.format_id
            && sample_rate == other.sample_rate
            && channels == other.channels;
}

bool sample_info_t::operator !=(const sample_info_t &other) const
{
    return ! operator == (other);
}

std::size_t sample_info_t::bps() const
{
    return audio_format_info_t::get_info(format_id).bps;
}

std::size_t sample_info_t::sample_size() const
{
    return (bps() * channels) / 8;
}

std::size_t sample_info_t::size_from_samples(std::size_t samples) const
{
    return samples * sample_size();
}

std::size_t sample_info_t::size_from_duration(timestamp_t duration) const
{
    return (duration * sample_rate * bps() * channels)
            / (8 * durations::seconds(1));
}

std::size_t sample_info_t::samples_from_size(std::size_t size) const
{
    if (auto ss = sample_size())
    {
        return size / ss;
    }

    return 0;
}

std::size_t sample_info_t::samples_from_duration(timestamp_t duration) const
{
    if (auto ss = sample_size())
    {
        return size_from_duration(duration) / ss;
    }

    return 0;
}

timestamp_t sample_info_t::duration_from_size(std::size_t size) const
{
    if (auto sd = size_from_duration(durations::seconds(1)))
    {
        return (size * durations::seconds(1))
                / sd;
    }

    return 0;
}

timestamp_t sample_info_t::duration_from_samples(std::size_t samples) const
{
    return duration_from_size(size_from_samples(samples));
}

bool sample_info_t::is_valid() const
{
    return audio_format_info_t::get_info(format_id).convertable
            && sample_rate > 0
            && channels > 0;
}


audio_sample_t::audio_sample_t(const sample_info_t& sample_info
                               , smart_buffer &&sample_data)
    : sample_info(sample_info)
    , sample_data(std::move(sample_data))
{

}

audio_sample_t::audio_sample_t(const sample_info_t& sample_info
                               , const smart_buffer &sample_data)
    : sample_info(sample_info)
    , sample_data(sample_data)
{

}

bool audio_sample_t::operator ==(const audio_sample_t &other) const
{
    return sample_info == other.sample_info
            && sample_data == other.sample_data;
}

bool audio_sample_t::operator !=(const audio_sample_t &other) const
{
    return ! operator == (other);
}

std::size_t audio_sample_t::samples() const
{
    return sample_info.samples_from_size(sample_data.size());
}

void audio_sample_t::resize(std::size_t new_samples)
{
    if (is_valid())
    {
        sample_data.resize(sample_info.size_from_samples(new_samples));
    }
}

const void *audio_sample_t::data() const
{
    return sample_data.data();
}

void *audio_sample_t::data()
{
    return sample_data.map();
}

bool audio_sample_t::clear()
{
    if (!sample_data.is_empty())
    {
        std::memset(sample_data.map()
                    , 0
                    , sample_data.size());
        return true;
    }

    return false;
}

bool audio_sample_t::is_valid() const
{
    return sample_info.is_valid()
            && sample_data.is_valid();
}

bool audio_sample_t::is_empty() const
{
    return sample_data.is_empty();
}

}
