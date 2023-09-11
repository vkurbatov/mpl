#include "apm_device_factory.h"
#include "core/enum_converter_defs.h"
#include "core/enum_serialize_defs.h"

#include "core/message_router_impl.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"


#include "audio_frame_impl.h"
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
                        | reader.get("aec.auto_delay_frames", wap_config.processing_config.aec_auto_delay_period)
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
                    && writer.set("aec.auto_delay_frames", wap_config.processing_config.aec_auto_delay_period)
                    && writer.set("gc.mode", wap_config.processing_config.gc_mode)
                    && writer.set("ns.mode", wap_config.processing_config.ns_mode)
                    && writer.set("vad.mode", wap_config.processing_config.vad_mode);
        }

        inline bool is_valid() const
        {
            return device_type == device_type_t::apm
                    && wap_config.is_valid();
        }

        inline bool is_compatible_format(const i_audio_format& audio_format) const
        {
            return audio_format.format_id() == audio_format_id_t::pcm16
                    && audio_format.channels() == wap_config.format.channels
                    && audio_format.sample_rate() == wap_config.format.sample_rate;
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    message_sink_impl           m_capture_sink;
    message_sink_impl           m_playback_sink;
    wap::wap_processor          m_native_device;

    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;

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
        , m_capture_sink([&](const auto& message) { return on_sink_message(message, true); })
        , m_playback_sink([&](const auto& message) { return on_sink_message(message, false); })
        , m_native_device(m_device_params.wap_config)
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_state(channel_state_t::ready)
    {

    }

    ~apm_device() override
    {

    }

    inline void change_state(channel_state_t new_state
                      , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }

    inline bool on_sink_message(const i_message& message, bool capture)
    {
        if (m_native_device.is_open())
        {
            if (message.category() == message_category_t::frame)
            {
                auto& media_frame = static_cast<const i_message_frame&>(message).frame();
                if (media_frame.media_type() == media_type_t::audio)
                {
                    return on_sink_frame(static_cast<const i_audio_frame&>(media_frame)
                                         , capture);
                }
            }
        }
        return false;
    }

    bool on_sink_frame(const i_audio_frame& audio_frame, bool capture)
    {
        if (m_device_params.is_compatible_format(audio_frame.format()))
        {
            if (auto buffer = audio_frame.buffers().get_buffer(media_buffer_index))
            {
                wap::sample_t sample(m_device_params.wap_config.format);

                sample.append_pcm16(buffer->data()
                                    , buffer->size() / 2);
                if (!sample.is_empty())
                {
                    return capture
                            ? on_capture_sample(std::move(sample))
                            : on_playback_sample(std::move(sample));
                }
            }
        }

        return false;
    }

    bool on_playback_sample(wap::sample_t&& sample)
    {
        return m_native_device.push_playback(sample.sample_data.data()
                                             , sample.samples());
    }

    bool on_capture_sample(wap::sample_t&& sample)
    {
        if (m_native_device.push_capture(sample.sample_data.data()
                                          , sample.samples()))
        {
            sample.clear();
            if (m_native_device.pop_result(sample))
            {
                if (auto samples = sample.samples())
                {
                    raw_array_t pcm_data(samples * 2);
                    if (sample.read_pcm16(pcm_data.data(), samples))
                    {
                        audio_format_impl audio_format(audio_format_id_t::pcm16
                                                       , sample.format.sample_rate
                                                       , sample.format.channels);

                        audio_frame_impl audio_frame(audio_format
                                                    , m_frame_counter
                                                    , m_frame_timestamp);

                        audio_frame.smart_buffers().set_buffer(media_buffer_index
                                                               , smart_buffer(std::move(pcm_data)));
                        m_frame_counter++;
                        m_frame_timestamp += samples;

                        message_frame_ref_impl message_frame(audio_frame);

                        return m_router.send_message(message_frame);
                    }

                }

            }

            return true;
        }

        return false;
    }

    inline bool open()
    {
        return m_native_device.open();
    }

    inline bool close()
    {
        return m_native_device.close();
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
                return &m_capture_sink;
            break;
            case 1:
                return &m_playback_sink;
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
    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        bool result = false;

        auto device_params = m_device_params;

        if (device_params.load(params)
                && device_params.is_valid())
        {
            if (!m_native_device.is_open())
            {
                m_device_params = device_params;
                m_native_device.set_config(device_params.wap_config);
                result = true;
            }
        }

        return result;
    }

    bool get_params(i_property &params) const override
    {
        return m_device_params.save(params);
    }
};

apm_device_factory::u_ptr_t apm_device_factory::create()
{
    return std::make_unique<apm_device_factory>();
}

apm_device_factory::apm_device_factory()
{

}

i_device::u_ptr_t apm_device_factory::create_device(const i_property &device_params)
{
    return apm_device::create(device_params);
}

}
