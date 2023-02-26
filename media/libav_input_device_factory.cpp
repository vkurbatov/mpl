#include "libav_input_device_factory.h"

#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/ffmpeg/libav_stream_grabber.h"
#include "tools/ffmpeg/libav_input_format.h"

#include <thread>
#include <shared_mutex>
#include <atomic>


namespace mpl::media
{

class libav_input_device : public i_device
{
    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    struct device_params_t
    {
        device_type_t   device_type = device_type_t::libav_in;
        std::string     url;
        std::string     options;

        device_params_t(device_type_t device_type = device_type_t::libav_in
                , const std::string_view& url = {}
                , const std::string_view& options = {})
            : device_type(device_type)
            , url(url)
            , options(options)
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
            return reader.get("url", url)
                    && reader.get("options", options);
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("url", url)
                    | writer.set("options", options);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::libav_in
                    && !url.empty();
        }

        ffmpeg::libav_input_format::config_t native_config() const
        {
            return { url
                     , options };
        }
    };

    device_params_t                 m_device_params;
    message_router_impl             m_router;

    frame_id_t                      m_frame_counter;
    timestamp_t                     m_frame_timestamp;

    timestamp_t                     m_start_time;

    std::thread                     m_thread;

    channel_state_t                 m_state;
    std::atomic_bool                m_open;

public:
    using u_ptr_t = std::unique_ptr<libav_input_device>;
    using s_ptr_t = std::shared_ptr<libav_input_device>;

    static u_ptr_t create(const i_property &device_params)
    {
        device_params_t libav_params(device_params);
        if (libav_params.is_valid())
        {
            return std::make_unique<libav_input_device>(std::move(libav_params));
        }

        return nullptr;
    }

    libav_input_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    ~libav_input_device()
    {
        close();
    }

    void change_state(channel_state_t new_state
                      , const std::string_view& reason = {})
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }

    bool open()
    {
        bool expected = false;
        if (m_open.compare_exchange_strong(expected
                                           , true
                                           , std::memory_order_acquire))
        {
            change_state(channel_state_t::opening);
            m_thread = std::thread([&]{ grabbing_thread(); });
            return true;
        }

        return false;
    }

    bool close()
    {
        if (m_open.load(std::memory_order_acquire))
        {

            change_state(channel_state_t::closing);

            m_open.store(false
                         , std::memory_order_release);
            if (m_thread.joinable())
            {
                m_thread.join();
            }

            change_state(channel_state_t::closed);

            return true;

        }

        return false;
    }

    void grabbing_thread()
    {
        change_state(channel_state_t::open);

        std::size_t error_counter = 0;

        enum class state_t
        {
            opening,
            reading,
            waiting
        };

        while(is_open())
        {
            ffmpeg::libav_input_format native_input_device(m_device_params.native_config());

            change_state(channel_state_t::connecting);

            if (native_input_device.open())
            {
                auto streams = native_input_device.streams();
                change_state(channel_state_t::connected);

                while(is_open()
                      && error_counter < 10)
                {
                    ffmpeg::frame_ref_t libav_frame;
                    if (native_input_device.read(libav_frame))
                    {
                        error_counter = 0;
                        on_native_frame(streams[libav_frame.info.stream_id]
                                        , libav_frame);
                    }
                    else
                    {
                        error_counter++;
                        if (is_open())
                        {
                            core::utils::sleep(durations::milliseconds(100));
                        }
                    }
                }

                change_state(channel_state_t::disconnecting);
                native_input_device.close();
                change_state(channel_state_t::disconnected);
            }
        }
    }

    void reset()
    {
        m_frame_counter = 0;
        m_frame_timestamp = 0;
        m_start_time = mpl::core::utils::now();
    }

    timestamp_t elapsed_time() const
    {
        return mpl::core::utils::now() - m_start_time;
    }


    bool on_native_frame(const ffmpeg::stream_info_t& stream_info
                         , ffmpeg::frame_ref_t& libav_frame)
    {
        if (libav_frame.size > 0)
        {
            switch(stream_info.media_info.media_type)
            {
                case ffmpeg::media_type_t::audio:
                {
                    audio_format_impl format;
                    if (core::utils::convert(stream_info
                                             , format))
                    {
                        audio_frame_impl frame(format
                                               , libav_frame.info.stream_id
                                               , libav_frame.info.timestamp());

                        frame.smart_buffers().set_buffer(main_media_buffer_index
                                                         , smart_buffer(libav_frame.data
                                                                        , libav_frame.size));

                        message_frame_ref_impl message_frame(frame);

                        m_router.send_message(message_frame);
                    }
                }
                break;
                case ffmpeg::media_type_t::video:
                {
                    video_format_impl format;
                    if (core::utils::convert(stream_info
                                             , format))
                    {
                        i_video_frame::frame_type_t frame_type = format.is_convertable()
                                ? i_video_frame::frame_type_t::undefined
                                : i_video_frame::frame_type_t::delta_frame;

                        if (libav_frame.info.key_frame)
                        {
                            frame_type = i_video_frame::frame_type_t::key_frame;
                        }

                        video_frame_impl frame(format
                                               , libav_frame.info.stream_id
                                               , libav_frame.info.timestamp()
                                               , frame_type);

                        frame.smart_buffers().set_buffer(main_media_buffer_index
                                                         , smart_buffer(libav_frame.data
                                                                        , libav_frame.size));

                        message_frame_ref_impl message_frame(frame);

                        m_router.send_message(message_frame);
                    }
                }
                break;
                default:;
            }
        }

        // нужно всегда возвращать true, иначе граббер будет копить входящий поток для отложенного чтения
        return true;
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
        return m_open.load(std::memory_order_acquire);
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_device interface
public:
    i_message_sink *sink() override
    {
        return nullptr;
    }

    i_message_source *source() override
    {
        return &m_router;
    }
    device_type_t device_type() const override
    {
        return device_type_t::v4l2_in;
    }
};

libav_input_device_factory::u_ptr_t libav_input_device_factory::create()
{
    return std::make_unique<libav_input_device_factory>();
}

libav_input_device_factory::libav_input_device_factory()
{

}

i_device::u_ptr_t libav_input_device_factory::create_device(const i_property &device_params)
{
    return libav_input_device::create(device_params);
}



}
