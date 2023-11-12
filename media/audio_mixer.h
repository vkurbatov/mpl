#ifndef MPL_AUDIO_MIXER_H
#define MPL_AUDIO_MIXER_H

//#include "audio_format_impl.h"
#include "audio_sample.h"

#include <vector>

namespace mpl::media
{


class audio_mixer
{
    using sample_data_t = std::vector<std::uint8_t>;

    audio_info_t           m_sample_info;
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

    audio_mixer(const audio_info_t& sample_info
                     , std::size_t buffer_size);

    void setup(const audio_info_t& sample_info
               , std::size_t buffer_size);

    const audio_info_t& sample_info() const;
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
