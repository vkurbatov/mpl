#include "audio_mixer.h"
#include "audio_format_helper.h"

#include <limits>
#include <cstring>

#include <math.h>
#include <algorithm>
#include <iostream>

namespace mpl::media
{

static constexpr double volume_log_base = 20.0;

namespace utils
{


template<typename T>
double get_level(const T& level)
{
    double min = static_cast<double>(std::numeric_limits<T>::min());
    double max = static_cast<double>(std::numeric_limits<T>::max());

    return ((static_cast<double>(level) - min)/(max - min) - 0.5) * 2;
}

template<>
double get_level(const double& level)
{
    return level;
}

template<>
double get_level(const float& level)
{
    return level;
}

double normalize_level(double base, double level)
{
    if (level > 0.0
            && level != 1.0)
    {
        //level = std::log(level)/std::log(base) + 1.0;
        level = std::pow(base, level) / base;
    }

    return level;
}

template<typename T>
inline std::size_t process(const void* input_samples
                           , void* output_samples
                           , std::size_t samples
                           , audio_mixer::mix_method_t operation
                           , double volume)
{
    const T* input = static_cast<const T*>(input_samples);
    T* output = static_cast<T*>(output_samples);

    double input_idx = 0.0;

    for (auto count = samples; count > 0; count--)
    {
        auto sample = input[static_cast<std::int32_t>(input_idx)];

        if (volume < 1.0)
        {
            sample = static_cast<double>(sample) * volume;
        }

        switch(operation)
        {
            case audio_mixer::mix_method_t::set:
                *output = sample;
            break;
            case audio_mixer::mix_method_t::mix:
                *output += sample;
            break;
            case audio_mixer::mix_method_t::demix:
                *output -= sample;
            break;
        }

        input_idx ++;
        output++;
    }

    return sizeof(T) * samples;
}

template<typename T>
void change_level(void *audio_samples
                  , std::size_t samples
                  , double level)
{
    level = normalize_level(volume_log_base, level);
    if (samples > 0)
    {
        T* audio = static_cast<T*>(audio_samples);
        for (std::size_t i = 0; i < samples; i++)
        {
            audio[i] = (static_cast<double>(audio[i]) * level);
        }
    }
}

}

double audio_mixer::normalize_volume(double volume)
{
    return utils::normalize_level(volume_log_base, volume);
}

void audio_mixer::change_audio_level(audio_format_id_t sample_format
                                        , void *audio_samples
                                        , std::size_t samples
                                        , double volume)
{
    switch(sample_format)
    {
        case audio_format_id_t::pcm8:
        case audio_format_id_t::pcm8p:
            return utils::change_level<std::int8_t>(audio_samples, samples, volume);
        break;
        case audio_format_id_t::pcm16:
        case audio_format_id_t::pcm16p:
           return utils::change_level<std::int16_t>(audio_samples, samples, volume);
        break;
        case audio_format_id_t::pcm32:
        case audio_format_id_t::pcm32p:
            return utils::change_level<std::int32_t>(audio_samples, samples, volume);
        break;
        case audio_format_id_t::float32:
        case audio_format_id_t::float32p:
            return utils::change_level<float>(audio_samples, samples, volume);
        break;
        case audio_format_id_t::float64:
        case audio_format_id_t::float64p:
            return utils::change_level<double>(audio_samples, samples, volume);
        break;
    }
}

std::size_t audio_mixer::process_samples(audio_format_id_t sample_format
                                          , const void *input_samples
                                          , void *output_samples
                                          , std::size_t samples
                                          , audio_mixer::mix_method_t method
                                          , double volume)
{
    if (volume == 1.0
            && method == audio_mixer::mix_method_t::set)
    {
        auto size = audio_format_helper::bytes_per_sample(sample_format) * samples;
        std::memcpy(output_samples
                    , input_samples
                    , size);

        return size;
    }

    switch(sample_format)
    {
        case audio_format_id_t::pcm8:
        case audio_format_id_t::pcm8p:
            return utils::process<std::int8_t>(input_samples, output_samples, samples, method, volume);
        break;
        case audio_format_id_t::pcm16:
        case audio_format_id_t::pcm16p:
           return utils::process<std::int16_t>(input_samples, output_samples, samples, method, volume);
        break;
        case audio_format_id_t::pcm32:
        case audio_format_id_t::pcm32p:
            return utils::process<std::int32_t>(input_samples, output_samples, samples, method, volume);
        break;
        case audio_format_id_t::float32:
        case audio_format_id_t::float32p:
            return utils::process<float>(input_samples, output_samples, samples, method, volume);
        break;
        case audio_format_id_t::float64:
        case audio_format_id_t::float64p:
            return utils::process<double>(input_samples, output_samples, samples, method, volume);
        break;
    }

    return 0;
}

audio_mixer::audio_mixer(const media::i_audio_format &audio_format
                           , std::size_t buffer_size)
    : m_audio_format(audio_format)
    , m_audio_data(audio_format_helper(audio_format).size_from_samples(buffer_size + 1))
    , m_sample_size(audio_format_helper(m_audio_format).sample_size())
    , m_write_cursor(0)
    , m_read_cursor(0)
    , m_overrun(0)
{

}

void audio_mixer::setup(const media::i_audio_format &audio_format
                             , std::size_t buffer_size)
{
    audio_format_helper helper(audio_format);
    auto data_size = helper.size_from_samples(buffer_size + 1);
    if (!m_audio_format.is_compatible(audio_format)
            || data_size != m_audio_data.size())
    {
        m_audio_format.assign(audio_format);
        m_audio_data.resize(helper.size_from_samples(buffer_size + 1));
        m_sample_size = helper.sample_size();
        reset();
    }
}

const media::i_audio_format &audio_mixer::format() const
{
    return m_audio_format;
}

std::size_t audio_mixer::pending() const
{
    return m_write_cursor - m_read_cursor;
}

std::size_t audio_mixer::capacity() const
{
    return audio_format_helper(m_audio_format).samples_from_size(m_audio_data.size());
}

std::size_t audio_mixer::overrun() const
{
    return m_overrun;
}

bool audio_mixer::is_empty() const
{
    return m_write_cursor == m_read_cursor;
}

std::size_t audio_mixer::push_data(const void* data
                                    , std::size_t samples
                                    , double volume)
{
    auto cap = capacity();
    if (samples < cap)
    {
        auto idx = m_write_cursor % cap;
        auto part = std::min(samples, cap - idx);

        if (volume != 1.0)
        {
            process_samples(m_audio_format.format_id()
                            , data
                            , m_audio_data.data() + idx * m_sample_size
                            , part * m_audio_format.channels()
                            , audio_mixer::mix_method_t::set
                            , utils::normalize_level(volume_log_base, volume));
        }
        else
        {
            std::memcpy(m_audio_data.data() + idx * m_sample_size
                        , data
                        , part * m_sample_size);
        }

        m_write_cursor += part;

        auto pen = pending();
        if (pen > cap)
        {
            m_overrun += pen - cap;
            m_read_cursor = m_write_cursor - cap + 1;
        }

        if (part < samples)
        {
            return push_data(static_cast<const std::uint8_t*>(data) + part * m_sample_size
                             , samples - part
                             , volume) + part;
        }

        return part;

    }
    return 0;
}

std::size_t audio_mixer::read_data(void *data
                                    , std::size_t samples
                                    , audio_mixer::mix_method_t operation
                                    , double volume) const
{
    if (samples <= pending())
    {
        auto cap = capacity();

        auto idx = m_read_cursor % cap;
        auto part = std::min(samples, cap - idx);
        volume = utils::normalize_level(volume_log_base, volume);

        process_samples(m_audio_format.format_id()
                        , m_audio_data.data() + idx * m_sample_size
                        , data
                        , part * m_audio_format.channels()
                        , operation
                        , volume);

        if (part < samples)
        {

            process_samples(m_audio_format.format_id()
                            , m_audio_data.data()
                            , static_cast<std::uint8_t*>(data) + part * m_sample_size
                            , (samples - part) * m_audio_format.channels()
                            , operation
                            , volume);

        }

        return samples;
    }

    return 0;
}

std::size_t audio_mixer::pop_data(void *data
                                   , std::size_t samples
                                   , audio_mixer::mix_method_t operation
                                   , double volume)
{
    auto result = read_data(data
                            , samples
                            , operation
                            , volume);
    drop(result);
    return result;

}

std::size_t audio_mixer::copy_data(audio_mixer &mixer
                                   , std::size_t samples
                                   , double volume)
{
    if (!m_audio_format.is_equal(mixer.m_audio_format))
    {
        return 0;
    }

    if (samples <= pending())
    {
        std::size_t result = 0;

        auto cap = capacity();

        auto idx = m_read_cursor % cap;
        auto part = std::min(samples, cap - idx);
        volume = utils::normalize_level(volume_log_base, volume);

        result += mixer.push_data(m_audio_data.data() + idx * m_sample_size
                                  , part);

        if (part < samples)
        {

            result += mixer.push_data(m_audio_data.data()
                                      , samples - part);

        }

        return result;
    }

    return 0;
}


bool audio_mixer::drop(std::size_t samples)
{
    if (m_read_cursor + samples <= m_write_cursor)
    {
        m_overrun = 0;
        m_read_cursor += samples;
        return true;
    }

    return false;
}

void audio_mixer::reset()
{
    m_write_cursor = 0;
    m_read_cursor = 0;
    m_overrun = 0;
}


void audio_mixer::sync_cursors()
{
    if (m_read_cursor > m_write_cursor)
    {
        m_write_cursor = m_read_cursor;
    }
    else if (pending() > capacity())
    {
        m_read_cursor = m_write_cursor - capacity() + 1;
    }
}

}
