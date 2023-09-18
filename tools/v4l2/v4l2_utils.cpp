#include "v4l2_utils.h"
#include "tools/base/string_base.h"

#include <linux/v4l2-controls.h>

#include <unordered_map>



namespace v4l2
{

template<typename Key, typename Value>
std::unordered_map<Value, Key> reverse_map(const std::unordered_map<Key, Value>& map)
{
    std::unordered_map<Value, Key> rev_map;
    for (const auto& [k,v]: map)
    {
        rev_map.emplace(v
                        , k);
    }

    return rev_map;
}

static const std::unordered_map<std::uint32_t, std::string> ctrl_map
{
    { V4L2_CID_EXPOSURE_AUTO                , "exposure_auto"                   },
    { V4L2_CID_EXPOSURE_ABSOLUTE            , "exposure_absolute"               },
    { V4L2_CID_EXPOSURE_AUTO_PRIORITY       , "exposure_auto_priority"          },
    { V4L2_CID_PAN_RELATIVE                 , "pan_relative"                    },
    { V4L2_CID_TILT_RELATIVE                , "tilt_relative"                   },
    { V4L2_CID_PAN_RESET                    , "pan_reset"                       },
    { V4L2_CID_TILT_RESET                   , "tilt_reset"                      },
    { V4L2_CID_PAN_ABSOLUTE                 , "pan_absolute"                    },
    { V4L2_CID_TILT_ABSOLUTE                , "tilt_absolute"                   },
    { V4L2_CID_FOCUS_ABSOLUTE               , "focus_absolute"                  },
    { V4L2_CID_FOCUS_RELATIVE               , "focus_relative"                  },
    { V4L2_CID_FOCUS_AUTO                   , "focus_auto"                      },
    { V4L2_CID_ZOOM_ABSOLUTE                , "zoom_absolute"                   },
    { V4L2_CID_ZOOM_RELATIVE                , "zoom_relative"                   },
    { V4L2_CID_ZOOM_CONTINUOUS              , "zoom_auto"                       },
    { V4L2_CID_PRIVACY                      , "privacy"                         },
    { V4L2_CID_IRIS_ABSOLUTE                , "iris_absolute"                   },
    { V4L2_CID_IRIS_RELATIVE                , "iris_relative"                   },
    { V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE  , "auto_n_present_while_balance"    },
    { V4L2_CID_WIDE_DYNAMIC_RANGE           , "wide_dynamic_range"              },
    { V4L2_CID_IMAGE_STABILIZATION          , "image_stabilization"             },
    { V4L2_CID_ISO_SENSITIVITY              , "iso_sensitivity"                 },
    { V4L2_CID_ISO_SENSITIVITY_AUTO         , "iso_sensitivity_auto"            },
    { V4L2_CID_EXPOSURE_METERING            , "exposure_metering"               },
    { V4L2_CID_SCENE_MODE                   , "scene_mode"                      },
    { V4L2_CID_3A_LOCK                      , "lock"                            },
    { V4L2_CID_AUTO_FOCUS_START             , "auto_focus_start"                },
    { V4L2_CID_AUTO_FOCUS_STOP              , "auto_focus_stop"                 },
    { V4L2_CID_AUTO_FOCUS_STATUS            , "auto_focus_status"               },
    { V4L2_CID_AUTO_FOCUS_RANGE             , "auto_focus_range"                },
    { V4L2_CID_PAN_SPEED                    , "pan_speed"                       },
    { V4L2_CID_TILT_SPEED                   , "tilt_speed"                      },
    { V4L2_CID_CAMERA_ORIENTATION           , "camera_orientation"              },
    { V4L2_CID_CAMERA_SENSOR_ROTATION       , "camera_sensor_rotation"          },
};

std::string get_ctrl_name(uint32_t ctrl_id)
{
    if (auto it = ctrl_map.find(ctrl_id); it != ctrl_map.end())
    {
        return it->second;
    }

    return std::to_string(ctrl_id);
}

uint32_t get_ctrl_id(const std::string_view &ctrl_name)
{
    static const auto reverse_ctrl_map = reverse_map(ctrl_map);

    if (auto it = reverse_ctrl_map.find(std::string(ctrl_name));
            it != reverse_ctrl_map.end())
    {
        return it->second;
    }

    return std::atol(&ctrl_name[0]);
}

std::string get_format_name(uint32_t format_id)
{
    std::string fourcc_string(reinterpret_cast<const char*>(&format_id), 4);

    return base::lower_string(fourcc_string);
}

uint32_t get_format_id(const std::string_view &format_name)
{
    if (format_name.size() == sizeof(std::uint32_t))
    {
        std::string fourcc_string = base::upper_string(format_name);

        return *reinterpret_cast<const std::uint32_t*>(fourcc_string.data());
    }

    return 0;
}



}
