#include "utils/enum_serialize_defs.h"

#include "device_types.h"
#include "media_types.h"
#include "audio_types.h"
#include "video_types.h"

namespace mpl
{

using namespace media;

declare_enum_serializer(device_type_t)
declare_enum_serializer(media_type_t)
declare_enum_serializer(audio_format_id_t)
declare_enum_serializer(video_format_id_t)

}
