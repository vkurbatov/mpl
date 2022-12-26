#ifndef MPL_AUDIO_TYPES_H
#define MPL_AUDIO_TYPES_H

namespace mpl
{

enum class audio_format_id_t
{
    undefined = -1,
    pcm8,
    pcm16,
    pcm32,
    float32,
    float64,
    pcm8p,
    pcm16p,
    pcm32p,
    float32p,
    float64p,
    pcma,
    pcmu,
    opus,
    aac
};

}

#endif // MPL_AUDIO_TYPES_H
