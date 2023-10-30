#ifndef MPL_MEDIA_AUDIO_INFO_H
#define MPL_MEDIA_AUDIO_INFO_H

#include "core/time_types.h"
#include "audio_types.h"

namespace mpl::media
{

class i_audio_format;

struct audio_info_t
{
    audio_format_id_t   format_id;
    std::uint32_t       sample_rate;
    std::uint32_t       channels;

    audio_info_t(audio_format_id_t format_id = audio_format_id_t::undefined
                  , std::uint32_t sample_rate = 0
                  , std::uint32_t channels = 0);

    audio_info_t(const i_audio_format& audio_format);

    bool operator == (const audio_info_t& other) const;
    bool operator != (const audio_info_t& other) const;

    std::size_t bps() const;
    std::size_t sample_size() const;

    std::size_t size_from_samples(std::size_t samples) const;
    std::size_t size_from_duration(timestamp_t duration) const;

    std::size_t samples_from_size(std::size_t size) const;
    std::size_t samples_from_duration(timestamp_t duration) const;

    timestamp_t duration_from_size(std::size_t size) const;
    timestamp_t duration_from_samples(std::size_t samples) const;

    bool is_valid() const;
};

}

#endif // MPL_MEDIA_AUDIO_INFO_H
