#ifndef MPL_MEDIA_AUDIO_FRAME_SPLITTER_H
#define MPL_MEDIA_AUDIO_FRAME_SPLITTER_H

#include "utils/data_splitter.h"
#include "audio_format_impl.h"

namespace mpl::media
{

class audio_frame_splitter
{
    using splitters_t = std::vector<data_splitter>;
    audio_format_impl       m_format;
    std::uint32_t           m_duration;
    splitters_t             m_splitters;
public:

    audio_frame_splitter();
    audio_frame_splitter(const i_audio_format& format
                         , std::uint32_t duration);

    void setup(const i_audio_format& format
               , std::uint32_t duration);

    void reset();

    const i_audio_format& format() const;
    std::uint32_t duration() const;
    std::size_t buffered_samples() const;

    data_fragment_queue_t push_frame(const void* data
                                    , std::size_t size);


};

}

#endif // MPL_MEDIA_AUDIO_FRAME_SPLITTER_H
