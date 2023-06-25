#ifndef MPL_AUDIO_FORMAT_HELPER_H
#define MPL_AUDIO_FORMAT_HELPER_H

#include "audio_types.h"
#include "core/time_types.h"

namespace mpl::media
{

class i_audio_format;

class audio_format_helper
{
    const i_audio_format&   m_audio_format;
public:

    static std::size_t bits_per_sample(audio_format_id_t format_id);
    static std::size_t bytes_per_sample(audio_format_id_t format_id);
    static bool is_encoded(audio_format_id_t format_id);
    static bool is_planar(audio_format_id_t format_id);

    audio_format_helper(const i_audio_format& audio_format);

    bool is_planar() const;
    std::size_t bits_per_sample() const;
    std::size_t sample_size() const;
    timestamp_t duration_form_size(std::size_t size) const;
    timestamp_t duration_form_samples(std::uint32_t samples) const;
    std::size_t size_from_duration(timestamp_t duration) const;
    std::size_t size_from_samples(std::uint32_t samples) const;
    std::uint32_t samples_from_size(std::uint32_t size) const;
    std::uint32_t samples_from_duration(timestamp_t duration) const;
};

}

#endif // MPL_AUDIO_FORMAT_HELPER_H
