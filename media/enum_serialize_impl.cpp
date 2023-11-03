#include "utils/enum_serialize_defs.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"

#include "tools/wap/wap_base.h"

namespace mpl
{

using namespace media;

__declare_enum_serializer(device_type_t)
__declare_enum_serializer(media_type_t)
__declare_enum_serializer(audio_format_id_t)
__declare_enum_serializer(video_format_id_t)

__declare_enum_serializer(pt::wap::echo_cancellation_mode_t)
__declare_enum_serializer(pt::wap::gain_control_mode_t)
__declare_enum_serializer(pt::wap::noise_suppression_mode_t)
__declare_enum_serializer(pt::wap::voice_detection_mode_t)

}
