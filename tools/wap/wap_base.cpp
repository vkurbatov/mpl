#include "wap_base.h"

namespace wap
{

sample_format_t::sample_format_t(uint32_t sample_rate
                                 , uint32_t channels)
    : sample_rate(sample_rate)
    , channels(channels)
{

}

bool sample_format_t::is_valid() const
{
    return sample_rate > 0
            && channels > 0;
}

std::size_t sample_format_t::bytes_per_sample() const
{
    return sample_size * channels;
}

std::size_t sample_format_t::samples_from_durations(std::size_t duration_ms) const
{
    if (is_valid())
    {
        return (duration_ms * sample_rate * channels) / 1000;
    }

    return 0;
}

std::size_t sample_format_t::duration_from_samples(std::size_t samples) const
{
    if (is_valid())
    {
        return (samples * 1000) / (sample_rate * channels);
    }

    return 0;
}

std::size_t sample_format_t::size_from_duration(std::size_t duration_ms) const
{
    return samples_from_durations(duration_ms) * bytes_per_sample();
}

std::size_t sample_format_t::size_from_samples(std::size_t samples) const
{
    return samples * bytes_per_sample();
}

std::size_t sample_format_t::samples_from_size(std::size_t size) const
{
    if (is_valid())
    {
        return size / bytes_per_sample();
    }

    return 0;
}

sample_t::sample_t(const sample_format_t &format
                   , const sample_data_t &sample_data)
    : format(format)
    , sample_data(sample_data)
{

}

sample_t::sample_t(const sample_format_t &format
                   , sample_data_t &&sample_data)
    : format(format)
    , sample_data(std::move(sample_data))
{

}

void sample_t::append_pcm16(const void *pcm, std::size_t samples)
{
   auto size = samples * sample_format_t::sample_size;
   auto src = static_cast<const std::int16_t*>(pcm);
   sample_data.resize(sample_data.size() + size);
   auto dst = reinterpret_cast<float*>(sample_data.data() + sample_data.size() - size);

   for (std::size_t i = 0; i < samples; i++)
   {
       dst[i] = static_cast<float>(src[i]) / 32768.0f;
   }
}

bool sample_t::read_pcm16(void *pcm, std::size_t samples) const
{
    if (this->samples() >= samples)
    {
        auto dst = static_cast<std::int16_t*>(pcm);
        auto src = reinterpret_cast<const float*>(sample_data.data());

        for (std::size_t i = 0; i < samples; i++)
        {
            dst[i] = static_cast<std::int16_t>(src[i] * 32768.0f);
        }

        return true;
    }

    return false;
}

std::size_t sample_t::samples() const
{
    return sample_data.size() / sample_format_t::sample_size;
}

bool sample_t::is_valid() const
{
    return format.is_valid()
            && !is_empty();
}

bool sample_t::is_empty() const
{
    return sample_data.empty();
}

void sample_t::clear()
{
    sample_data.clear();
}

processing_config_t::processing_config_t(std::int32_t ap_delay_offset_ms
                                         , std::int32_t ap_delay_stream_ms
                                         , echo_cancellation_mode_t aec_mode
                                         , uint32_t aec_drift_ms
                                         , std::uint32_t aec_auto_delay_period
                                         , gain_control_mode_t gc_mode
                                         , noise_suppression_mode_t ns_mode
                                         , voice_detection_mode_t vad_mode)
    : ap_delay_offset_ms(ap_delay_offset_ms)
    , ap_delay_stream_ms(ap_delay_stream_ms)
    , aec_mode(aec_mode)
    , aec_drift_ms(aec_drift_ms)
    , aec_auto_delay_period(aec_auto_delay_period)
    , gc_mode(gc_mode)
    , ns_mode(ns_mode)
    , vad_mode(vad_mode)
{

}


}
