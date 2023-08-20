#include "wap_processor.h"

#include <webrtc/modules/audio_processing/include/audio_processing.h>
#include <webrtc/modules/interface/module_common_types.h>

#include <queue>
#include <iostream>

namespace wap
{

namespace detail
{

using frame_queue_t = std::queue<sample_data_t>;

struct delay_metrics_t
{
    std::int32_t    median_delay;
    std::int32_t    std_delay;
    float           fraction_poor_delays;

    delay_metrics_t(std::int32_t median_delay = -1
                    , std::int32_t std_delay = -1
                    , float fraction_poor_delays = -1.0f)
        : median_delay(median_delay)
        , std_delay(std_delay)
        , fraction_poor_delays(fraction_poor_delays)
    {

    }

    inline bool is_valid() const
    {
        return median_delay != -1
                || std_delay != -1
                || fraction_poor_delays != -1.0f;
    }

    inline bool operator == (const delay_metrics_t& other) const
    {
        return median_delay == other.median_delay
                && std_delay == other.std_delay
                && fraction_poor_delays == other.fraction_poor_delays;
    }

    inline bool operator != (const delay_metrics_t& other) const
    {
        return ! operator == (other);
    }

    inline void reset()
    {
        *this = {};
    }
};

class frame_splitter
{
    sample_data_t   m_current_frame;
    frame_queue_t   m_frames;
    std::size_t     m_delay;
public:
    frame_splitter(std::size_t fragment_size
                   , std::size_t delay = 0)
        : m_delay(delay)
    {
        m_current_frame.reserve(fragment_size);
        if (delay > 0)
        {
            for (auto i = 0; i < delay; i++)
            {
                sample_data_t data(fragment_size);
                m_frames.emplace(std::move(data));
            }
        }
    }

    bool push_frame(const void* data
                    , std::size_t size)
    {
        bool result = false;

        const auto* ptr = static_cast<const std::uint8_t*>(data);

        while(size > 0)
        {
            auto need_size = m_current_frame.capacity() - m_current_frame.size();
            auto part_size = std::min(need_size, size);

            if (part_size > 0)
            {
                m_current_frame.insert(m_current_frame.end()
                                       , ptr
                                       , ptr + part_size);

                if (part_size == need_size)
                {
                    result |= true;
                    m_frames.push(m_current_frame);
                    m_current_frame.clear();
                }
            }

            ptr += part_size;
            size -= part_size;
        }


        return result;
    }

    inline std::size_t fragment_size() const
    {
        return m_current_frame.capacity();
    }

    inline std::size_t pending_frames() const
    {
        return m_frames.size();
    }

    inline frame_queue_t fetch_frames()
    {
        if (m_delay > 0)
        {
            frame_queue_t result;
            while(m_frames.size() > m_delay)
            {
                result.emplace(std::move(m_frames.front()));
                m_frames.pop();
            }

            return result;
        }

        return std::move(m_frames);
    }

    inline void reset()
    {
        m_current_frame.clear();
        m_frames = {};
    }

    inline void reset(std::size_t fragment_size)
    {
        m_current_frame.clear();

        if (m_current_frame.capacity() > fragment_size)
        {
            m_current_frame.shrink_to_fit();
        }

        m_current_frame.reserve(fragment_size);
        m_frames = {};
    }

};

void set_ap_params(webrtc::AudioProcessing& ap
                   , const wap_processor::config_t& config)
{

    // audio processing
    ap.set_delay_offset_ms(config.processing_config.ap_delay_offset_ms);
    ap.set_stream_delay_ms(config.processing_config.ap_delay_stream_ms);

    // echo cancellation
    if (config.processing_config.aec_mode == echo_cancellation_mode_t::none)
    {
        ap.echo_cancellation()->Enable(false);
    }
    else
    {
        ap.echo_cancellation()->Enable(true);
        ap.echo_cancellation()->set_stream_drift_samples(config.format.samples_from_durations(config.processing_config.aec_drift_ms));
        ap.echo_cancellation()->enable_drift_compensation(config.processing_config.aec_drift_ms > 0);
        ap.echo_cancellation()->set_suppression_level(static_cast<webrtc::EchoCancellation::SuppressionLevel>(config.processing_config.aec_mode));
        ap.echo_cancellation()->enable_delay_logging(config.processing_config.aec_auto_delay_gain > 0);
    }

    // gain control
    if (config.processing_config.gc_mode == gain_control_mode_t::none)
    {
        ap.gain_control()->Enable(false);
    }
    else
    {
        ap.gain_control()->set_mode(static_cast<webrtc::GainControl::Mode>(config.processing_config.gc_mode));
        ap.gain_control()->Enable(true);
    }

    // noise suppression
    if (config.processing_config.ns_mode == noise_suppression_mode_t::none)
    {
        ap.noise_suppression()->Enable(false);
    }
    else
    {
        ap.noise_suppression()->set_level(static_cast<webrtc::NoiseSuppression::Level>(config.processing_config.ns_mode));
        ap.noise_suppression()->Enable(true);
    }

    // voice detection
    if (config.processing_config.vad_mode == voice_detection_mode_t::none)
    {
        ap.voice_detection()->Enable(false);
    }
    else
    {
        ap.voice_detection()->set_likelihood(static_cast<webrtc::VoiceDetection::Likelihood>(config.processing_config.vad_mode));
        ap.voice_detection()->Enable(true);
    }
}

void get_ap_params(const webrtc::AudioProcessing& ap
                   , wap_processor::config_t& config)
{
    // echo cancellation
    if (ap.echo_cancellation()->is_enabled())
    {
        config.processing_config.aec_mode = static_cast<echo_cancellation_mode_t>(ap.echo_cancellation()->suppression_level());
        if (ap.echo_cancellation()->is_drift_compensation_enabled())
        {
            config.processing_config.aec_drift_ms = config.format.duration_from_samples(ap.echo_cancellation()->stream_drift_samples());
        }
        else
        {
            config.processing_config.aec_drift_ms = 0;
        }
    }
    else
    {
        config.processing_config.aec_mode = echo_cancellation_mode_t::none;
        config.processing_config.aec_drift_ms = 0;
    }

    // gain control
    if (ap.gain_control()->is_enabled())
    {
        config.processing_config.gc_mode = static_cast<gain_control_mode_t>(ap.gain_control()->mode());
    }
    else
    {
        config.processing_config.gc_mode = gain_control_mode_t::none;
    }

    // noise suppression
    if (ap.noise_suppression()->is_enabled())
    {
        config.processing_config.ns_mode = static_cast<noise_suppression_mode_t>(ap.noise_suppression()->level());
    }
    else
    {
        config.processing_config.ns_mode = noise_suppression_mode_t::none;
    }

    // voice detection
    if (ap.voice_detection()->is_enabled())
    {
        config.processing_config.vad_mode = static_cast<voice_detection_mode_t>(ap.voice_detection()->likelihood());
    }
    else
    {
        config.processing_config.vad_mode = voice_detection_mode_t::none;
    }
}

}

struct wap_processor::pimpl_t
{
    using native_ap_ptr_t = std::unique_ptr<webrtc::AudioProcessing>;

    using u_ptr_t = wap_processor::pimpl_ptr_t;
    using config_t = wap_processor::config_t;

    static constexpr std::size_t fragment_duration_ms = 10;

    config_t                m_config;
    webrtc::StreamConfig    m_native_stream_config;
    native_ap_ptr_t         m_native_ap;
    detail::delay_metrics_t m_delay_metrics;
    std::size_t             m_playback_frames;
    std::size_t             m_record_frames;

    detail::frame_splitter  m_playback_splitter;
    detail::frame_splitter  m_record_splitter;

    sample_t                m_output_sample;

    static webrtc::StreamConfig create_native_stream_config(const sample_format_t& sample_format)
    {
        return { static_cast<std::int32_t>(sample_format.sample_rate)
                    , static_cast<std::int32_t>(sample_format.channels) };
    }

    static webrtc::ProcessingConfig create_native_config(const webrtc::StreamConfig& stream_config)
    {
        return { stream_config
                , stream_config
                , stream_config
                , stream_config
               };
    }

    static webrtc::ProcessingConfig create_native_config(const sample_format_t& sample_format)
    {
        return create_native_config(create_native_stream_config(sample_format));
    }

    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<pimpl_t>(config);
    }

    pimpl_t(const config_t& config)
        : m_config(config)
        , m_native_stream_config(create_native_stream_config(m_config.format))
        , m_playback_frames(0)
        , m_record_frames(0)
        , m_playback_splitter(config.format.size_from_samples(m_native_stream_config.num_frames())
                              , 0)
        , m_record_splitter(config.format.size_from_samples(m_native_stream_config.num_frames())
                            , 20)
        , m_output_sample(config.format)
    {

    }

    ~pimpl_t()
    {

    }

    bool open()
    {
        if (m_native_ap == nullptr)
        {
            m_native_ap.reset(webrtc::AudioProcessing::Create());
            if (m_native_ap)
            {
                auto result = m_native_ap->Initialize(create_native_config(m_native_stream_config));
                if (result == webrtc::AudioProcessing::kNoError)
                {

                    reset();
                    detail::set_ap_params(*m_native_ap
                                          , m_config);
                    /*detail::get_ap_params(*m_native_ap
                                          , m_config);*/
                    return true;
                }
            }

        }

        return false;
    }

    bool close()
    {
        if (m_native_ap != nullptr)
        {
            m_native_ap.reset();
            return true;
        }

        return false;
    }

    inline bool is_open() const
    {
        return m_native_ap != nullptr;
    }

    inline const config_t& config() const
    {
        return m_config;
    }

    inline bool set_config(const config_t& config)
    {
        if (config.is_valid())
        {
            m_config = config;
            return true;
        }

        return false;
    }

    bool push_playback(const void* data
                       , std::size_t samples)
    {
        bool result = false;

        if (is_open())
        {
            auto data_size = m_config.format.size_from_samples(samples);
            if (m_playback_splitter.push_frame(data
                                               , data_size))
            {


                auto frames = m_playback_splitter.fetch_frames();
                while(!frames.empty())
                {
                    auto* float_ptr = reinterpret_cast<float*>(frames.front().data());

                    auto webrtc_result = m_native_ap->ProcessReverseStream(&float_ptr
                                                                          , m_native_stream_config
                                                                          , m_native_stream_config
                                                                          , &float_ptr);
                    if (webrtc_result == webrtc::AudioProcessing::kNoError)
                    {
                        m_playback_frames++;
                        result |= true;
                    }

                    std::cout << "Push playback: size: " << frames.front().size()
                              << ", result: " << webrtc_result << std::endl;

                    frames.pop();
                }
            }
        }

        return result;
    }

    bool push_record(const void* data
                     , std::size_t samples)
    {
        bool result = false;

        if (is_open())
        {
            auto data_size = m_config.format.size_from_samples(samples);
            if (m_record_splitter.push_frame(data
                                             , data_size))
            {
                auto frames = m_record_splitter.fetch_frames();
                while(!frames.empty())
                {
                    auto metrics = m_delay_metrics;
                    auto has_echo = m_native_ap->echo_cancellation()->stream_has_echo();

                    if (m_config.processing_config.aec_mode != echo_cancellation_mode_t::none)
                    {
                        auto delay = m_native_ap->stream_delay_ms();

                        if (m_config.processing_config.aec_auto_delay_gain > 0)
                        {
                            if (m_record_frames != 0
                                    && m_record_frames % 100 == 0)
                            {
                                if (m_native_ap->echo_cancellation()->GetDelayMetrics(&metrics.median_delay
                                                                                      , &metrics.std_delay
                                                                                      , &metrics.fraction_poor_delays) == 0)
                                {
                                    if (metrics.is_valid()
                                            && m_delay_metrics != metrics)
                                    {

                                        delay += metrics.median_delay
                                                * m_config.processing_config.aec_auto_delay_gain
                                                * metrics.fraction_poor_delays;
                                        m_delay_metrics = metrics;
                                    }
                                }
                            }
                        }

                        m_native_ap->set_stream_delay_ms(delay);

                        if (m_config.processing_config.aec_drift_ms > 0)
                        {
                            m_native_ap->echo_cancellation()->set_stream_drift_samples(m_config.format.samples_from_durations(m_config.processing_config.aec_drift_ms));
                        }
                    }


                    auto* float_ptr = reinterpret_cast<float*>(frames.front().data());
                    auto webrtc_result = m_native_ap->ProcessStream(&float_ptr
                                                                    , m_native_stream_config
                                                                    , m_native_stream_config
                                                                    , &float_ptr);

                    std::cout << "Push record: size: " << frames.front().size()
                              << ", result: " << webrtc_result
                              << ", stream_delay: " << m_native_ap->stream_delay_ms()
                              << ", median_delay: " << m_delay_metrics.median_delay
                              << ", standart_delay: " << m_delay_metrics.std_delay
                              << ", fraction_poor_delays: " << m_delay_metrics.fraction_poor_delays
                              << ", has_echo: " << has_echo
                              << std::endl;

                    if (webrtc_result == webrtc::AudioProcessing::kNoError)
                    {
                        m_record_frames++;
                        m_output_sample.sample_data.insert(m_output_sample.sample_data.end()
                                                           , frames.front().begin()
                                                           , frames.front().end());
                        result |= true;
                    }

                    frames.pop();
                }
            }

        }

        return result;
    }

    bool pop_result(sample_t& sample)
    {
        if (!m_output_sample.is_empty())
        {
            sample = std::move(m_output_sample);
            return true;
        }

        return false;
    }

    std::size_t get_stream_delay_ms() const
    {
        if (m_native_ap)
        {
            return m_native_ap->stream_delay_ms();
        }

        return 0;
    }

    void reset()
    {
        m_playback_splitter.reset();
        m_record_splitter.reset();
        m_playback_frames = 0;
        m_record_frames = 0;
    }
};

wap_processor::config_t::config_t(const sample_format_t &format
                                  , const processing_config_t& processing_config)
    : format(format)
    , processing_config(processing_config)
{

}

bool wap_processor::config_t::is_valid() const
{
    return format.is_valid();
}



wap_processor::wap_processor(const config_t& config)
    : m_pimpl(pimpl_t::create(config))
{

}

wap_processor::~wap_processor()
{

}

const wap_processor::config_t &wap_processor::config() const
{
    return m_pimpl->config();
}

bool wap_processor::set_config(const config_t &config)
{
    return m_pimpl->set_config(config);
}

bool wap_processor::push_playback(const void *data, std::size_t samples)
{
    return m_pimpl->push_playback(data, samples);
}

bool wap_processor::push_record(const void *data, std::size_t samples)
{
    return m_pimpl->push_record(data, samples);
}

bool wap_processor::pop_result(sample_t &sample)
{
    return m_pimpl->pop_result(sample);
}

std::size_t wap_processor::get_stream_delay_ms() const
{
    return m_pimpl->get_stream_delay_ms();
}

bool wap_processor::open()
{
    return m_pimpl->open();
}

bool wap_processor::close()
{
    return m_pimpl->close();
}

bool wap_processor::is_open() const
{
    return m_pimpl->is_open();
}

}
