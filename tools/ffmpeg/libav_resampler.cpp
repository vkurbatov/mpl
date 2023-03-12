#include "libav_resampler.h"
extern "C"
{
#include <libavutil/opt.h>
#include <libavutil/channel_layout.h>
#include <libavutil/samplefmt.h>
#include "libswresample/swresample.h"
}

#include <cstring>
#include <iostream>

namespace ffmpeg
{

struct swr_wrapper
{
    SwrContext*     m_swr_context = nullptr;
    audio_info_t    m_input_format;
    audio_info_t    m_output_format;

    swr_wrapper()
    {

    }

    ~swr_wrapper()
    {
        close();
    }

    bool check_or_reopen(const audio_info_t& input_format
                        , const audio_info_t& output_format)
    {
        if (m_swr_context != nullptr)
        {
            if (m_input_format != input_format
                    || m_output_format != output_format)
            {
                return close()
                        && check_or_reopen(input_format
                                         , output_format);
            }
        }
        else
        {
            m_swr_context = swr_alloc_set_opts(m_swr_context
                                               , av_get_default_channel_layout(output_format.channels)
                                               , static_cast<AVSampleFormat>(output_format.sample_format)
                                               , output_format.sample_rate
                                               , av_get_default_channel_layout(input_format.channels)
                                               , static_cast<AVSampleFormat>(input_format.sample_format)
                                               , input_format.sample_rate
                                               , 0
                                               , nullptr);

            if (m_swr_context != nullptr
                    && swr_init(m_swr_context) >= 0)
            {
                m_input_format = input_format;
                m_output_format = output_format;
            }
            else
            {
                close();
            }

            /*
            if (m_swr_context != nullptr)
            {
                bool result =
                       av_opt_set_int(m_swr_context, "in_channel_layout",       av_get_default_channel_layout(input_format.channels), 0) >= 0
                    && av_opt_set_int(m_swr_context, "in_sample_rate",          input_format.sample_rate, 0) >= 0
                    && av_opt_set_sample_fmt(m_swr_context, "in_sample_fmt",    static_cast<AVSampleFormat>(input_format.sample_format), 0) >= 0

                    && av_opt_set_int(m_swr_context, "out_channel_layout",      av_get_default_channel_layout(output_format.channels), 0) >= 0
                    && av_opt_set_int(m_swr_context, "out_sample_rate",         output_format.sample_rate, 0) >= 0
                    && av_opt_set_sample_fmt(m_swr_context, "out_sample_fmt",   static_cast<AVSampleFormat>(output_format.sample_format), 0) >= 0;

                result = result && swr_init(m_swr_context) >= 0;

                if (result)
                {
                    m_input_format = input_format;
                    m_output_format = output_format;
                }
                else
                {
                    close();
                }
            }*/
        }

        return m_swr_context != nullptr;
    }


    media_data_t resample(const void* input_data
                         , std::size_t input_size)
    {
        if (is_open())
        {
            auto input_samples = m_input_format.sample_size() != 0 ? input_size / m_input_format.sample_size() : 0;

            if (input_samples != 0)
            {

                static const auto max_channels = 32;

                std::uint8_t* input_buffers[max_channels];
                std::uint8_t* output_buffers[max_channels];

                auto output_samples = av_rescale_rnd(input_samples
                                                     , m_output_format.sample_rate
                                                     , m_input_format.sample_rate
                                                     , AV_ROUND_UP);

                media_data_t output_data(output_samples * m_output_format.sample_size());

                auto ret1 = av_samples_fill_arrays(input_buffers
                                       , nullptr
                                       , reinterpret_cast<const std::uint8_t*>(input_data)
                                       , m_input_format.channels
                                       , input_samples
                                       , static_cast<AVSampleFormat>(m_input_format.sample_format)
                                       , 1);

                auto ret2 = av_samples_fill_arrays(output_buffers
                                       , nullptr
                                       , static_cast<const std::uint8_t*>(output_data.data())
                                       , m_output_format.channels
                                       , output_samples
                                       , static_cast<AVSampleFormat>(m_output_format.sample_format)
                                       , 1);
                if (ret1 >= 0 && ret2 >= 0)
                {
                    auto result = swr_convert(m_swr_context
                                              , output_buffers
                                              , output_samples
                                              , const_cast<const uint8_t**>(input_buffers)
                                              , input_samples);

                    if (result < 0)
                    {
                        return {};
                    }

                    if (output_samples != result)
                    {
                        output_data.resize(result * m_output_format.sample_size());
                    }

                    return output_data;
                }
            }
        }

        return {};
    }

    bool close()
    {
        if (m_swr_context != nullptr)
        {
            swr_free(&m_swr_context);
            m_swr_context = nullptr;
            return true;
        }

        return false;
    }

    bool is_open() const
    {
        return m_swr_context != nullptr;
    }
};

struct resampler_context_t
{
    swr_wrapper     m_swr_resampler;
    resampler_context_t()
    {

    }

    ~resampler_context_t()
    {
        m_swr_resampler.close();
    }


    media_data_t resample(const audio_info_t& input_format
                  , const void* input_data
                  , std::size_t input_size
                  , const audio_info_t& output_format)
    {
        if (input_data != nullptr)
        {
            if (input_format == output_format)
            {
                return media_data_t(static_cast<const std::uint8_t*>(input_data)
                                    , static_cast<const std::uint8_t*>(input_data) + input_size);
            }
            if (m_swr_resampler.check_or_reopen(input_format, output_format))
            {
                return m_swr_resampler.resample(input_data
                                                , input_size);
            }
        }

        return {};
    }

};

libav_resampler::libav_resampler()
    : m_resampler_context(std::make_unique<resampler_context_t>())
{

}

libav_resampler::~libav_resampler()
{

}

media_data_t libav_resampler::resample(const audio_info_t &input_format
                               , const void *input_data
                               , std::size_t input_size
                               , const audio_info_t &output_format)
{
    return m_resampler_context->resample(input_format
                                         , input_data
                                         , input_size
                                         , output_format);
}

}
