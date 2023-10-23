#include "audio_sample.h"
#include "audio_info.h"
#include "i_audio_format.h"

#include <cstring>

namespace mpl::media
{

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
    if (sample_info.is_valid())
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
