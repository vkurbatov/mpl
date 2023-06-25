#ifndef MPL_AUDIO_MIXER_H
#define MPL_AUDIO_MIXER_H

#include "audio_format_impl.h"

#include <vector>

namespace mpl::media
{

class audio_mixer
{
    using sample_data_t = std::vector<std::uint8_t>;

    audio_format_impl       m_audio_format;
    sample_data_t           m_audio_data;
    std::size_t             m_sample_size;
    std::size_t             m_write_cursor;
    std::size_t             m_read_cursor;
    std::size_t             m_overrun;
public:

    enum class mix_method_t
    {
        set,
        mix,
        demix
    };

    static double normalize_volume(double volume);

    static void change_audio_level(audio_format_id_t sample_format
                                   , void* audio_samples
                                   , std::size_t samples
                                   , double volume);

    static std::size_t process_samples(audio_format_id_t sample_format
                                       , const void* input_samples
                                       , void* output_samples
                                       , std::size_t samples
                                       , mix_method_t method = mix_method_t::set
                                       , double volume = 1.0);

    audio_mixer(const i_audio_format& audio_format
                     , std::size_t buffer_size);

    void setup(const i_audio_format& audio_format
               , std::size_t buffer_size);

    const i_audio_format& format() const;
    std::size_t pending() const;
    std::size_t capacity() const;
    std::size_t overrun() const;
    bool is_empty() const;


    std::size_t push_data(const void* data
                          , std::size_t samples
                          , double volume = 1.0);


    std::size_t read_data(void* data
                          , std::size_t samples
                          , mix_method_t method = mix_method_t::set
                          , double volume = 1.0) const;


    std::size_t pop_data(void* data
                          , std::size_t samples
                          , mix_method_t method = mix_method_t::set
                          , double volume = 1.0);

    std::size_t copy_data(audio_mixer& mixer
                          , std::size_t samples
                          , double volume = 1.0);

    bool drop(std::size_t samples);

    void reset();
private:
    void sync_cursors();
};

}

#endif // MPL_AUDIO_MIXER_H
