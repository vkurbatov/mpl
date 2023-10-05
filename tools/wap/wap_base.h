#ifndef WAP_BASE_H
#define WAP_BASE_H

#include <vector>
#include <cstdint>

namespace wap
{

using sample_data_t = std::vector<std::uint8_t>;
enum class echo_cancellation_mode_t
{
    none = -1,
    low,
    moderation,
    high
};

enum class gain_control_mode_t
{
    none = -1,
    adaptive_analog,
    adaptive_digital,
    fixed_digital
};

enum class noise_suppression_mode_t
{
    none = -1,
    low,
    moderate,
    high,
    very_high
};

enum class voice_detection_mode_t
{
    none = -1,
    very_low,
    low,
    moderate,
    high
};

struct processing_config_t
{
    std::int32_t                ap_delay_offset_ms;
    std::int32_t                ap_delay_stream_ms;
    echo_cancellation_mode_t    aec_mode;
    std::uint32_t               aec_drift_ms;
    std::uint32_t               aec_auto_delay_period;
    gain_control_mode_t         gc_mode;
    noise_suppression_mode_t    ns_mode;
    voice_detection_mode_t      vad_mode;

    processing_config_t(std::int32_t ap_delay_offset_ms = 0
                        , std::int32_t ap_delay_stream_ms = 0
                        , echo_cancellation_mode_t aec_mode = echo_cancellation_mode_t::none
                        , std::uint32_t aec_drift_ms = 0
                        , std::uint32_t aec_auto_delay_gain = 0
                        , gain_control_mode_t gc_mode = gain_control_mode_t::none
                        , noise_suppression_mode_t ns_mode = noise_suppression_mode_t::none
                        , voice_detection_mode_t vad_mode = voice_detection_mode_t::none);
};

struct sample_format_t
{
    static constexpr std::size_t sample_size = sizeof(float);
    std::uint32_t   sample_rate;
    std::uint32_t   channels;

    sample_format_t(std::uint32_t sample_rate = 0
                    , std::uint32_t channels = 0);

    bool is_valid() const;

    std::size_t bytes_per_sample() const;
    std::size_t samples_from_durations(std::size_t duration_ms) const;
    std::size_t duration_from_samples(std::size_t samples) const ;
    std::size_t size_from_duration(std::size_t duration_ms) const;
    std::size_t size_from_samples(std::size_t samples) const;
    std::size_t samples_from_size(std::size_t size) const;
};

struct sample_t
{
    sample_format_t     format;
    sample_data_t       sample_data;

    sample_t(const sample_format_t& format
             , const sample_data_t& sample_data);

    sample_t(const sample_format_t& format = {}
            , sample_data_t&& sample_data = {});

    void append_pcm16(const void* pcm, std::size_t samples);
    bool read_pcm16(void* pcm, std::size_t samples) const;

    std::size_t samples() const;
    bool is_valid() const;
    bool is_empty() const;

    void clear();

};



}

#endif // WAP_BASE_H
