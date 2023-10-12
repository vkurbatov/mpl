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
    { V4L2_CID_BRIGHTNESS                   , "brightness"                      },
    { V4L2_CID_CONTRAST                     , "contrast"                        },
    { V4L2_CID_SATURATION                   , "saturation"                      },
    { V4L2_CID_HUE                          , "hue"                             },
    { V4L2_CID_AUDIO_VOLUME                 , "audio_volume"                    },
    { V4L2_CID_AUDIO_BALANCE                , "audio_balance"                   },
    { V4L2_CID_AUDIO_BASS                   , "audio_bass"                      },
    { V4L2_CID_AUDIO_TREBLE                 , "audio_treble"                    },
    { V4L2_CID_AUDIO_MUTE                   , "audio_mute"                      },
    { V4L2_CID_AUDIO_LOUDNESS               , "audio_loudness"                  },
    { V4L2_CID_BLACK_LEVEL                  , "black_level"                     },
    { V4L2_CID_AUTO_WHITE_BALANCE           , "auto_white_balance"              },
    { V4L2_CID_DO_WHITE_BALANCE             , "do_white_balance"                },
    { V4L2_CID_RED_BALANCE                  , "red_balance"                     },
    { V4L2_CID_BLUE_BALANCE                 , "blue_balance"                    },
    { V4L2_CID_GAMMA                        , "gamma"                           },
    { V4L2_CID_WHITENESS                    , "whiteness"                       },
    { V4L2_CID_EXPOSURE                     , "explosure"                       },
    { V4L2_CID_AUTOGAIN                     , "autogain"                        },
    { V4L2_CID_GAIN                         , "gain"                            },
    { V4L2_CID_HFLIP                        , "hflip"                           },
    { V4L2_CID_VFLIP                        , "vflip"                           },
    { V4L2_CID_POWER_LINE_FREQUENCY         , "power_line_frequence"            },
    { V4L2_CID_HUE_AUTO                     , "hue_auto"                        },
    { V4L2_CID_WHITE_BALANCE_TEMPERATURE    , "white_balance_temperature"       },
    { V4L2_CID_SHARPNESS                    , "sharpness"                       },
    { V4L2_CID_BACKLIGHT_COMPENSATION       , "blacklight_compensation"         },
    { V4L2_CID_CHROMA_AGC                   , "chroma_agc"                      },
    { V4L2_CID_COLOR_KILLER                 , "color_killer"                    },
    { V4L2_CID_COLORFX                      , "colorfx"                         },
    { V4L2_CID_AUTOBRIGHTNESS               , "autobrightness"                  },
    { V4L2_CID_BAND_STOP_FILTER             , "band_stop_filter"                },
    { V4L2_CID_ROTATE                       , "rotate"                          },
    { V4L2_CID_BG_COLOR                     , "bg_color"                        },
    { V4L2_CID_CHROMA_GAIN                  , "chroma_gain"                     },
    { V4L2_CID_ILLUMINATORS_1               , "illuminators_1"                  },
    { V4L2_CID_ILLUMINATORS_2               , "illuminators_2"                  },
    { V4L2_CID_MIN_BUFFERS_FOR_CAPTURE      , "min_buffers_for_capture"         },
    { V4L2_CID_MIN_BUFFERS_FOR_OUTPUT       , "min_buffers_for_output"          },
    { V4L2_CID_ALPHA_COMPONENT              , "alpha_component"                 },
    { V4L2_CID_COLORFX_CBCR                 , "colorfx_cbcr"                    },

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

    return portable::lower_string(fourcc_string);
}

uint32_t get_format_id(const std::string_view &format_name)
{
    if (format_name.size() == sizeof(std::uint32_t))
    {
        std::string fourcc_string = portable::upper_string(format_name);

        return *reinterpret_cast<const std::uint32_t*>(fourcc_string.data());
    }

    return 0;
}



}
