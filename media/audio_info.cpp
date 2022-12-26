#include "audio_info.h"

namespace mpl
{

namespace detail
{

const audio_format_info_t audio_format_info_table[] =
{
    { -1,  -1,  false, false, false },    // undefined
    { 8,   0,   false, false, true },     // pcm8
    { 16,  0,   false, false, true },     // pcm16
    { 32,  0,   false, false, true },     // pcm32
    { 32,  0,   false, false, true },     // float32
    { 64,  0,   false, false, true },     // float64
    { 8,   0,   false, true, true },      // pcm8p
    { 16,  0,   false, true, true },      // pcm16p
    { 32,  0,   false, true, true },      // pcm32p
    { 32,  0,   false, true, true },      // float32p
    { 64,  0,   false, true, true },      // float64p
    { 8,  20,   true, false, false },     // pcma
    { 8,  20,   true, false, false },     // pcmu
    { 16,  20,   true, false, false },    // opus
    { 32,  1024, true, false, false }     // aac
};

}

const audio_format_info_t &audio_format_info_t::get_info(audio_format_id_t format_id)
{
    return detail::audio_format_info_table[static_cast<std::int32_t>(format_id)];
}



}
