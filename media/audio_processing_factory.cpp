#include "audio_processing_factory.h"
#include "core/enum_converter_defs.h"
#include "core/enum_serialize_defs.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "tools/wap/wap_processor.h"

namespace mpl::core::utils
{

declare_enum_converter_begin(wap::echo_cancellation_mode_t)
    declare_pair(wap::echo_cancellation_mode_t, none),
    declare_pair(wap::echo_cancellation_mode_t, low),
    declare_pair(wap::echo_cancellation_mode_t, moderation),
    declare_pair(wap::echo_cancellation_mode_t, high)
declare_enum_converter_end(wap::echo_cancellation_mode_t)

declare_enum_converter_begin(wap::gain_control_mode_t)
    declare_pair(wap::gain_control_mode_t, none),
    declare_pair(wap::gain_control_mode_t, adaptive_analog),
    declare_pair(wap::gain_control_mode_t, adaptive_digital),
    declare_pair(wap::gain_control_mode_t, fixed_digital)
declare_enum_converter_end(wap::gain_control_mode_t)

declare_enum_converter_begin(wap::noise_suppression_mode_t)
    declare_pair(wap::noise_suppression_mode_t, none),
    declare_pair(wap::noise_suppression_mode_t, low),
    declare_pair(wap::noise_suppression_mode_t, moderate),
    declare_pair(wap::noise_suppression_mode_t, high),
    declare_pair(wap::noise_suppression_mode_t, very_high)
declare_enum_converter_end(wap::noise_suppression_mode_t)

declare_enum_converter_begin(wap::voice_detection_mode_t)
    declare_pair(wap::voice_detection_mode_t, none),
    declare_pair(wap::voice_detection_mode_t, very_low),
    declare_pair(wap::voice_detection_mode_t, low),
    declare_pair(wap::voice_detection_mode_t, moderate),
    declare_pair(wap::voice_detection_mode_t, high)
declare_enum_converter_end(wap::voice_detection_mode_t)

}

namespace mpl
{

declare_enum_serializer(wap::echo_cancellation_mode_t)
declare_enum_serializer(wap::gain_control_mode_t)
declare_enum_serializer(wap::noise_suppression_mode_t)
declare_enum_serializer(wap::voice_detection_mode_t)

}

namespace mpl::media
{

class apm_device : public i_device
{
    using u_ptr_t = std::unique_ptr<apm_device>;

    struct device_params_t
    {
        device_type_t                   device_type;
        wap::wap_processor::config_t    wap_config;

        device_params_t(device_type_t device_type = device_type_t::apm
                        , const wap::wap_processor::config_t& wap_config = {})
            : device_type(device_type)
            , wap_config(wap_config)
        {

        }

        device_params_t(const i_property& params)
            : device_params_t()
        {
            load(params);
        }

        bool load(const i_property& params)
        {
            property_reader reader(params);
            if (reader.get("device_type", device_type_t::apm) == device_type_t::apm)
            {
                return reader.get("format.sample_rate", wap_config.format.sample_rate)
                        | reader.get("format.channels", wap_config.format.channels)
                        | reader.get("delay_offset_ms", wap_config.processing_config.ap_delay_offset_ms)
                        | reader.get("delay_stream_ms", wap_config.processing_config.ap_delay_stream_ms)
                        | reader.get("aec.mode", wap_config.processing_config.aec_mode)
                        | reader.get("aec.drift_ms", wap_config.processing_config.aec_drift_ms)
                        | reader.get("aec.auto_delay_gain", wap_config.processing_config.aec_auto_delay_gain)
                        | reader.get("gc.mode", wap_config.processing_config.gc_mode)
                        | reader.get("ns.mode", wap_config.processing_config.ns_mode)
                        | reader.get("vad.mode", wap_config.processing_config.vad_mode);
            }

            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::apm)
                    && writer.set("format.channels", wap_config.format.channels)
                    && writer.set("delay_offset_ms", wap_config.processing_config.ap_delay_offset_ms)
                    && writer.set("delay_stream_ms", wap_config.processing_config.ap_delay_stream_ms)
                    && writer.set("aec.mode", wap_config.processing_config.aec_mode)
                    && writer.set("aec.drift_ms", wap_config.processing_config.aec_drift_ms)
                    && writer.set("aec.auto_delay_gain", wap_config.processing_config.aec_auto_delay_gain)
                    && writer.set("gc.mode", wap_config.processing_config.gc_mode)
                    && writer.set("ns.mode", wap_config.processing_config.ns_mode)
                    && writer.set("vad.mode", wap_config.processing_config.vad_mode);
        }

        inline bool is_valid() const
        {
            return device_type == device_type_t::apm
                    && wap_config.is_valid();
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    message_sink_impl           m_capture_sink;
    message_sink_impl           m_playback_sink;
    wap::wap_processor          m_native_device;

    channel_state_t             m_state;

public:

    static u_ptr_t create(const i_property &params)
    {
        device_params_t apm_params(params);
        if (apm_params.is_valid())
        {
            return std::make_unique<apm_device>(std::move(apm_params));
        }

        return nullptr;
    }

    apm_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_capture_sink([&](const auto& message) { return on_send_capture_message(message); })
        , m_playback_sink([&](const auto& message) { return on_send_playback_message(message); })
        , m_native_device(m_device_params.wap_config)
        , m_state(channel_state_t::ready)
    {

    }

    ~apm_device() override
    {

    }

    bool on_send_capture_message(const i_message& message)
    {
        return false;
    }

    bool on_send_playback_message(const i_message& message)
    {
        return false;
    }

    bool open()
    {
        return m_native_device.open();
    }

    bool close()
    {
        return m_native_device.close();
    }

    bool set_params(const i_property& input_params)
    {
        bool result = false;

        auto device_params = m_device_params;

        if (device_params.load(input_params)
                && device_params.is_valid())
        {
            if (!m_native_device.is_open())
            {
                m_device_params = device_params;
                result = true;
            }
        }

        return result;
    }

    bool get_params(i_property& output_params)
    {
        if (m_device_params.save(output_params))
        {
            return true;
        }

        return false;
    }

    bool internal_configure(const i_property* input_params
                            , i_property* output_params)
    {
        bool result = false;

        if (input_params != nullptr)
        {
            result = set_params(*input_params);
        }

        if (output_params != nullptr)
        {
            result = get_params(*output_params);
        }

        return result;
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:
                return open();
            break;
            case channel_control_id_t::close:
                return close();
            break;
            case channel_control_id_t::configure:
                return internal_configure(control.input_params
                                          , control.output_params);
            break;
            default:;
        }

        return false;
    }

    bool is_open() const override
    {
         return m_native_device.is_open();
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        switch(index)
        {
            case 0:

            break;
            case 1:

            break;
            default:;
        }

        return nullptr;
    }

    i_message_source *source(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_router;
        }

        return nullptr;
    }

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::apm;
    }
};

audio_processing_factory::u_ptr_t audio_processing_factory::create()
{
    return std::make_unique<audio_processing_factory>();
}

audio_processing_factory::audio_processing_factory()
{

}

i_device::u_ptr_t audio_processing_factory::create_device(const i_property &device_params)
{
    return apm_device::create(device_params);
}

}
