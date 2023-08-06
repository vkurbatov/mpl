#include "wap_processor.h"

#include <webrtc/modules/audio_processing/include/audio_processing.h>
#include <webrtc/modules/interface/module_common_types.h>

namespace wap
{

namespace detail
{

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

    static webrtc::StreamConfig create_native_stream_config(const sample_format_t& sample_format)
    {
        return { sample_format.sample_rate, sample_format.channels };
    }

    static webrtc::ProcessingConfig create_native_config(const sample_format_t& sample_format)
    {
        webrtc::StreamConfig stream_config = create_native_stream_config(sample_format);
        return { stream_config
                , stream_config
                , stream_config
                , stream_config
               };
    }

    config_t            m_config;
    native_ap_ptr_t     m_native_ap;

    static u_ptr_t create(const config_t& config)
    {
        return nullptr;
    }

    pimpl_t(const config_t& config)
        : m_config(config)
    {

    }

    ~pimpl_t()
    {

    }

    bool push_playback(const void* data
                       , std::size_t samples)
    {
        return false;
    }

    bool push_record(const void* data
                     , std::size_t samples)
    {
        return false;
    }

    bool pop_playback(sample_t& sample)
    {
        return false;
    }

    bool init()
    {
        m_native_ap.reset(webrtc::AudioProcessing::Create());
        if (m_native_ap)
        {
            auto result = m_native_ap->Initialize(create_native_config(m_config.format));
            if (result == webrtc::AudioProcessing::kNoError)
            {

            }
        }

        return false;
    }

    void reset()
    {

    }
};

wap_processor::config_t::config_t(const sample_format_t &format)
    : format(format)
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

bool wap_processor::push_playback(const void *data, std::size_t samples)
{
    return m_pimpl->push_playback(data, samples);
}

bool wap_processor::push_record(const void *data, std::size_t samples)
{
    return m_pimpl->push_record(data, samples);
}

bool wap_processor::pop_playback(sample_t &sample)
{
    return m_pimpl->pop_playback(sample);
}

void wap_processor::reset()
{
    return m_pimpl->reset();
}

}
