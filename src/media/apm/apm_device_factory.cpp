#include "apm_device_factory.h"
#include "apm_device_params.h"

#include "utils/message_router_impl.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "utils/time_utils.h"
#include "utils/enum_utils.h"

#include "core/event_channel_state.h"

#include "media/audio_frame_impl.h"

#include "tools/wap/wap_processor.h"

#include "log/log_tools.h"

namespace mpl::media
{

class apm_device : public i_device
{
    using u_ptr_t = std::unique_ptr<apm_device>;



    apm_device_params_t         m_device_params;
    message_router_impl         m_router;
    message_sink_impl           m_capture_sink;
    message_sink_impl           m_playback_sink;
    pt::wap::wap_processor      m_native_device;

    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;

    channel_state_t             m_state;

public:

    static u_ptr_t create(const i_property &params)
    {
        apm_device_params_t apm_params;
        if (utils::property::deserialize(apm_params
                                         , params)
                && apm_params.is_valid())
        {
            return std::make_unique<apm_device>(std::move(apm_params));
        }

        return nullptr;
    }

    apm_device(apm_device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_capture_sink([&](const auto& message) { return on_sink_message(message, true); })
        , m_playback_sink([&](const auto& message) { return on_sink_message(message, false); })
        , m_native_device(m_device_params.wap_config)
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_state(channel_state_t::ready)
    {
        mpl_log_info("apm_device #", this, " init {", m_device_params.wap_config.format.sample_rate
                     , ", ", m_device_params.wap_config.format.channels, "}");
    }

    ~apm_device() override
    {
        mpl_log_info("apm_device #", this, " destruction");
    }

    inline void change_state(channel_state_t new_state
                      , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            mpl_log_info("apm_device #", this, ": state: ", utils::enum_to_string(m_state), "->", utils::enum_to_string(new_state));
            m_state = new_state;
            m_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }


    inline bool is_compatible_format(const i_audio_format& audio_format) const
    {
        return audio_format.format_id() == audio_format_id_t::pcm16
                && audio_format.channels() == static_cast<std::int32_t>(m_device_params.wap_config.format.channels)
                && audio_format.sample_rate() == static_cast<std::int32_t>(m_device_params.wap_config.format.sample_rate);
    }

    inline bool on_sink_message(const i_message& message, bool capture)
    {
        if (m_native_device.is_open())
        {
            mpl_log_debug("apm_device #", this, ": on_message(", utils::enum_to_string(message.category()), ", ", capture, ")");

            if (message.category() == message_category_t::data)
            {
                auto& media_frame = static_cast<const i_media_frame&>(message);
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

        if (is_compatible_format(audio_frame.format()))
        {
            if (auto buffer = audio_frame.data().get_buffer(media_buffer_index))
            {
                pt::wap::sample_t sample(m_device_params.wap_config.format);

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
        else
        {
            mpl_log_debug("apm_device #", this, ": on_frame: incompatible format");
        }

        return false;
    }

    bool on_playback_sample(pt::wap::sample_t&& sample)
    {
        mpl_log_debug("apm_device #", this, ": on_playback_sample(", sample.samples(), ")");
        return m_native_device.push_playback(sample.sample_data.data()
                                             , sample.samples());
    }

    bool on_capture_sample(pt::wap::sample_t&& sample)
    {
        mpl_log_debug("apm_device #", this, ": on_capture_sample(", sample.samples(), ")");
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

                         mpl_log_debug("apm_device #", this, ": send result sample(", samples, ")");

                        return m_router.send_message(audio_frame);
                    }
                }
            }
            else
            {
                mpl_log_debug("apm_device #", this, ": can't pop result sample");
            }

            return true;
        }
        else
        {
            mpl_log_debug("apm_device #", this, ": can't push capture sample");
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

        if (utils::property::deserialize(device_params
                                         , params)
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
        return utils::property::serialize(m_device_params
                                          , params);
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
