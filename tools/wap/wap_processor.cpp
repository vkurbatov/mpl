#include "wap_processor.h"

#include <webrtc/modules/audio_processing/include/audio_processing.h>
#include <webrtc/modules/interface/module_common_types.h>

#include <queue>

namespace wap
{

namespace detail
{

using frame_queue_t = std::queue<sample_data_t>;

class frame_splitter
{
    sample_data_t   m_current_frame;
    frame_queue_t   m_frames;
public:
    frame_splitter(std::size_t fragment_size)
    {
        m_current_frame.reserve(fragment_size);
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
                   , const processing_config_t& config)
{

    // audio processing
    ap.set_delay_offset_ms(config.ap_delay_offset_ms);
    ap.set_stream_delay_ms(config.ap_delay_stream_ms);

    // echo cancellation
    if (config.aec_mode == echo_cancellation_mode_t::none)
    {
        ap.echo_cancellation()->Enable(false);
    }
    else
    {
        ap.echo_cancellation()->set_stream_drift_samples(config.aec_drift);
        ap.echo_cancellation()->enable_drift_compensation(config.aec_drift > 0);
        ap.echo_cancellation()->set_suppression_level(static_cast<webrtc::EchoCancellation::SuppressionLevel>(config.aec_mode));
        ap.echo_cancellation()->Enable(true);
    }

    // gain control
    if (config.gc_mode == gain_control_mode_t::none)
    {
        ap.gain_control()->Enable(false);
    }
    else
    {
        ap.gain_control()->set_mode(static_cast<webrtc::GainControl::Mode>(config.gc_mode));
        ap.gain_control()->Enable(true);
    }

    // noise suppression
    if (config.ns_mode == noise_suppression_mode_t::none)
    {
        ap.noise_suppression()->Enable(false);
    }
    else
    {
        ap.noise_suppression()->set_level(static_cast<webrtc::NoiseSuppression::Level>(config.ns_mode));
        ap.noise_suppression()->Enable(true);
    }

    // voice detection
    if (config.vad_mode == voice_detection_mode_t::none)
    {
        ap.voice_detection()->Enable(false);
    }
    else
    {
        ap.voice_detection()->set_likelihood(static_cast<webrtc::VoiceDetection::Likelihood>(config.vad_mode));
        ap.voice_detection()->Enable(true);
    }
}

void get_ap_params(const webrtc::AudioProcessing& ap
                   , processing_config_t& config)
{
    // echo cancellation
    if (ap.echo_cancellation()->is_enabled())
    {
        config.aec_mode = static_cast<echo_cancellation_mode_t>(ap.echo_cancellation()->suppression_level());
        if (ap.echo_cancellation()->is_drift_compensation_enabled())
        {
            config.aec_drift = ap.echo_cancellation()->stream_drift_samples();
        }
        else
        {
            config.aec_drift = 0;
        }
    }
    else
    {
        config.aec_mode = echo_cancellation_mode_t::none;
        config.aec_drift = 0;
    }

    // gain control
    if (ap.gain_control()->is_enabled())
    {
        config.gc_mode = static_cast<gain_control_mode_t>(ap.gain_control()->mode());
    }
    else
    {
        config.gc_mode = gain_control_mode_t::none;
    }

    // noise suppression
    if (ap.noise_suppression()->is_enabled())
    {
        config.ns_mode = static_cast<noise_suppression_mode_t>(ap.noise_suppression()->level());
    }
    else
    {
        config.ns_mode = noise_suppression_mode_t::none;
    }

    // voice detection
    if (ap.voice_detection()->is_enabled())
    {
        config.vad_mode = static_cast<voice_detection_mode_t>(ap.voice_detection()->likelihood());
    }
    else
    {
        config.vad_mode = voice_detection_mode_t::none;
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
        , m_playback_splitter(config.format.size_from_duration(fragment_duration_ms))
        , m_record_splitter(config.format.size_from_duration(fragment_duration_ms))
        , m_output_sample(config.format)
    {

    }

    ~pimpl_t()
    {

    }

    bool init()
    {
        m_native_ap.reset(webrtc::AudioProcessing::Create());
        if (m_native_ap)
        {
            auto result = m_native_ap->Initialize(create_native_config(m_native_stream_config));
            if (result == webrtc::AudioProcessing::kNoError)
            {
                detail::set_ap_params(*m_native_ap
                                      , m_config.processing_config);
                detail::get_ap_params(*m_native_ap
                                      , m_config.processing_config);
                return true;
            }
        }

        return false;
    }

    inline bool has_init() const
    {
        return m_native_ap != nullptr;
    }

    inline const config_t& config() const
    {
        return m_config;
    }

    bool push_playback(const void* data
                       , std::size_t samples)
    {
        bool result = false;

        if (m_playback_splitter.push_frame(data
                                           , samples))
        {
            auto frames = m_playback_splitter.fetch_frames();
            while(!frames.empty())
            {
                auto* float_ptr = reinterpret_cast<float*>(frames.front().data());
                auto webrtc_result = m_native_ap->ProcessReverseStream(&float_ptr
                                                                      , m_native_stream_config
                                                                      , m_native_stream_config
                                                                      , &float_ptr);
                frames.pop();

                result |= webrtc_result == webrtc::AudioProcessing::kNoError;
            }
        }

        return result;
    }

    bool push_record(const void* data
                     , std::size_t samples)
    {
        bool result = false;

        if (m_record_splitter.push_frame(data
                                         , samples))
        {
            auto frames = m_record_splitter.fetch_frames();
            while(!frames.empty())
            {
                auto* float_ptr = reinterpret_cast<float*>(frames.front().data());
                auto webrtc_result = m_native_ap->ProcessStream(&float_ptr
                                                                , m_native_stream_config
                                                                , m_native_stream_config
                                                                , &float_ptr);

                if (webrtc_result == webrtc::AudioProcessing::kNoError)
                {
                    m_output_sample.sample_data.insert(m_output_sample.sample_data.end()
                                                       , frames.front().begin()
                                                       , frames.front().end());
                    result |= true;
                }

                frames.pop();
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

    void reset()
    {
        m_playback_splitter.reset();
        m_record_splitter.reset();
        init();
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

void wap_processor::reset()
{
    return m_pimpl->reset();
}

}
