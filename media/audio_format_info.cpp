#include "audio_format_info.h"

#include "format_utils.h"

#include <string>
#include <unordered_map>

namespace mpl::media
{

namespace detail
{

const audio_format_info_t audio_format_info_table[] =
{
    //bps   fs      enc     planar  conv
    { -1,  -1,      false,  false,  false,  utils::to_fourcc({})  },    // undefined
    { 8,    0,      false,  false,  true,   utils::to_fourcc({})  },    // pcm8
    { 16,   0,      false,  false,  true,   utils::to_fourcc({})  },    // pcm16
    { 32,   0,      false,  false,  true,   utils::to_fourcc({})  },    // pcm32
    { 32,   0,      false,  false,  true,   utils::to_fourcc({})  },    // float32
    { 64,   0,      false,  false,  true,   utils::to_fourcc({})  },    // float64
    { 8,    0,      false,  true,   true,   utils::to_fourcc({})  },    // pcm8p
    { 16,   0,      false,  true,   true,   utils::to_fourcc({})  },    // pcm16p
    { 32,   0,      false,  true,   true,   utils::to_fourcc({})  },    // pcm32p
    { 32,   0,      false,  true,   true,   utils::to_fourcc({})  },    // float32p
    { 64,   0,      false,  true,   true,   utils::to_fourcc({})  },    // float64p
    { 8,    20,     true,   false,  false,  utils::to_fourcc({})  },    // pcma
    { 8,    20,     true,   false,  false,  utils::to_fourcc({})  },    // pcmu
    { 16,   20,     true,   false,  false,  utils::to_fourcc({})  },    // opus
    { 32,   1024,   true,   false,  false,  utils::to_fourcc({})  }     // aac
};

}

const audio_format_info_t &audio_format_info_t::get_info(audio_format_id_t format_id)
{
    return detail::audio_format_info_table[static_cast<std::int32_t>(format_id) + 1];
}

}
