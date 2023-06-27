#include "audio_sample.h"
#include "audio_info.h"

#include <cstring>

namespace mpl::media
{

audio_sample_t::audio_sample_t(audio_format_id_t format_id
                               , uint32_t sample_rate
                               , uint32_t channels
                               , smart_buffer &&sample_data)
    : format_id(format_id)
    , sample_rate(sample_rate)
    , channels(channels)
    , sample_data(std::move(sample_data))
{

}

audio_sample_t::audio_sample_t(audio_format_id_t format_id
                               , uint32_t sample_rate
                               , uint32_t channels
                               , const smart_buffer &sample_data)
    : format_id(format_id)
    , sample_rate(sample_rate)
    , channels(channels)
    , sample_data(sample_data)
{

}

bool audio_sample_t::operator ==(const audio_sample_t &other) const
{
    return format_id == other.format_id
            && sample_rate == other.sample_rate
            && channels == other.channels
            && sample_data == other.sample_data;
}

bool audio_sample_t::operator !=(const audio_sample_t &other) const
{
    return ! operator == (other);
}

std::size_t audio_sample_t::samples() const
{
    if (is_valid())
    {
        return ((sample_data.size() / channels * 8)
                / audio_format_info_t::get_info(format_id).bps);
    }

    return 0;
}

void audio_sample_t::resize(std::size_t new_samples)
{
    if (is_valid())
    {

        sample_data.resize((new_samples
                           * audio_format_info_t::get_info(format_id).bps
                           * channels) / 8);
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
    return audio_format_info_t::get_info(format_id).convertable
            && sample_rate > 0
            && channels > 0
            && sample_data.is_valid();
}

bool audio_sample_t::is_empty() const
{
    return sample_data.is_empty();
}

}
