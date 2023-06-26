#ifndef MPL_AUDIO_SAMPLE_H
#define MPL_AUDIO_SAMPLE_H

#include "core/smart_buffer.h"
#include "audio_types.h"

namespace mpl::media
{

struct audio_sample_t
{
    audio_format_id_t   format_id;
    std::uint32_t       sample_rate;
    std::uint32_t       channels;

    smart_buffer        sample_data;

    audio_sample_t(audio_format_id_t format_id = audio_format_id_t::undefined
                 , std::uint32_t sample_rate = 0
                 , std::uint32_t channels = 0
                 , smart_buffer&& sample_data = {});

    audio_sample_t(audio_format_id_t format_id
                   , std::uint32_t sample_rate
                   , std::uint32_t channels
                   , const smart_buffer& sample_data);

    bool operator == (const audio_sample_t& other) const;
    bool operator != (const audio_sample_t& other) const;

    std::size_t samples() const;

    const void* pixels() const;
    void* pixels();

    bool tune();

    bool is_valid() const;
    bool is_empty() const;

    audio_sample_t();
};

}

#endif // MPL_AUDIO_SAMPLE_H
