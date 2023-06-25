#include "audio_level.h"

#include "i_audio_format.h"
#include "audio_format_helper.h"

#include <limits>
#include <algorithm>

namespace mpl::media
{

namespace detail
{

template<typename T>
T max_level()
{
    return static_cast<double>(std::numeric_limits<T>::max());
}

template<typename T>
T min_level()
{
    return static_cast<double>(std::numeric_limits<T>::min());
}

template<>
float max_level()
{
    return 1.0f;
}

template<>
float min_level()
{
    return 0.0f;
}

template<>
double max_level()
{
    return 1.0;
}

template<>
double min_level()
{
    return 0.0;
}

template<typename T>
T range()
{
    return max_level<T>() - min_level<T>();
}

template<typename T>
T max_abs_value(const void* data
                , std::size_t samples)
{
    const T* ptr = static_cast<const T*>(data);
    T result = min_level<T>();
    while(samples-- > 0)
    {
        result = std::max<T>(result, *ptr);
        ptr++;
    }

    return result;
}

template<typename T>
double to_double(const T& value)
{
    return (static_cast<double>(value))
            / static_cast<double>(max_level<T>());
}

template<>
double to_double(const double& value)
{
    return value;
}

template<>
double to_double(const float& value)
{
    return value;
}

double max_abs_value(audio_format_id_t sample_format
                     , const void* data
                     , std::size_t samples)
{
    switch(sample_format)
    {
        case audio_format_id_t::pcm8:
        case audio_format_id_t::pcm8p:
            return to_double(max_abs_value<std::int8_t>(data
                                                        , samples));
        break;
        case audio_format_id_t::pcm16:
        case audio_format_id_t::pcm16p:
            return to_double(max_abs_value<std::int16_t>(data
                                                         , samples));
        break;
        case audio_format_id_t::pcm32:
        case audio_format_id_t::pcm32p:
            return to_double(max_abs_value<std::int32_t>(data
                                                        , samples));
        break;
        case audio_format_id_t::float32:
        case audio_format_id_t::float32p:
            return max_abs_value<float>(data
                                        , samples);
        break;
        case audio_format_id_t::float64:
        case audio_format_id_t::float64p:
            return max_abs_value<double>(data
                                         , samples);
        break;
    }

    return 0.0;
}

}

audio_level::config_t::config_t(timestamp_t update_frequency)
    : update_frequency(update_frequency)
{

}


audio_level::audio_level(const config_t& config)
    : m_config(config)
{
    reset();
}

bool audio_level::push_frame(const i_media_format &format
                             , const void *data
                             , std::size_t size)
{
    if (format.media_type() == media_type_t::audio
            && format.is_convertable()
            && size > 0)
    {
        const i_audio_format& audio_format = static_cast<const i_audio_format&>(format);
        audio_format_helper helper(audio_format);

        auto samples = helper.samples_from_size(size);
        auto duration = helper.duration_form_samples(samples);

        if (samples > 0)
        {
            auto abs_value = detail::max_abs_value(audio_format.format_id()
                                                   , data
                                                   , samples);


            m_abs_max = std::max(abs_value, m_abs_max);

            if (m_count++ == m_config.update_frequency)
            {
                m_level = m_abs_max;

                m_count = 0;

                m_abs_max /= 4;
            }

            double double_duration = static_cast<double>(duration)/durations::milliseconds(1);

            double addition_energy = m_level;
            addition_energy *= addition_energy;
            m_total_energy += addition_energy * double_duration;
            m_total_duration += duration;

            return true;
        }
    }
    return false;
}

void audio_level::reset(bool only_level)
{
    m_abs_max = 0.0;
    m_count = 0;
    m_level = 0.0;
    if (!only_level)
    {
        m_total_energy = 0.0;
        m_total_duration = 0;
    }
}

double audio_level::max_level() const
{
    return m_abs_max;
}

double audio_level::level() const
{
    return m_level;
}

double audio_level::total_energy() const
{
    return m_total_energy;
}

timestamp_t audio_level::total_duration() const
{
    return m_total_duration;
}

}
