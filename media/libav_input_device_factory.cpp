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
        device_type_t   device_type = device_type_t::libav;
        std::string     url;
        std::string     options;

        device_params_t(device_type_t device_type = device_type_t::libav
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
                    | reader.get("options", options);
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("url", url)
                    || writer.set("options", options) ;
        }

        bool is_valid() const
        {
            return device_type == device_type_t::libav
                    && !url.empty();
        }

        ffmpeg::libav_grabber_config_t native_config() const
        {
            return { url, ffmpeg::stream_mask_only_media, options };
        }
    };


    device_params_t                 m_device_params;
    message_router_impl             m_router;
    ffmpeg::libav_stream_grabber    m_native_device;

    frame_id_t                      m_frame_counter;
    timestamp_t                     m_frame_timestamp;

    timestamp_t                     m_start_time;

    channel_state_t                 m_state;
    std::atomic_bool                m_open;

public:
    using u_ptr_t = std::unique_ptr<libav_input_device>;
    using s_ptr_t = std::shared_ptr<libav_input_device>;

    static u_ptr_t create(const i_property &device_params)
    {
        return nullptr;
    }

    libav_input_device()
    {

    }

    ~libav_input_device()
    {

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
            reset();

            change_state(channel_state_t::opening);

            if (m_native_device.open(m_device_params.native_config()))
            {
                return true;
            }

            m_open.store(false
                         , std::memory_order_release);
            change_state(channel_state_t::failed);
        }

        return false;
    }

    bool close()
    {
        if (m_open.load(std::memory_order_acquire))
        {
            change_state(channel_state_t::closing);

            if (!m_native_device.close())
            {
                change_state(channel_state_t::failed);
            }

            m_open.store(false
                         , std::memory_order_release);

            change_state(channel_state_t::closed);

            return true;

        }

        return false;
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
                         , ffmpeg::frame_t&& libav_frame)
    {
        if (!libav_frame.media_data.empty())
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
                                               , libav_frame.info.id
                                               , libav_frame.info.timestamp());

                        frame.smart_buffers().set_buffer(main_media_buffer_index
                                                         , smart_buffer(std::move(libav_frame.media_data)));

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
                        video_frame_impl frame(format
                                               , libav_frame.info.id
                                               , libav_frame.info.timestamp());

                        frame.smart_buffers().set_buffer(main_media_buffer_index
                                                         , smart_buffer(std::move(libav_frame.media_data)));

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

        return true;
    }

    void on_native_device_state(const ffmpeg::streaming_event_t& streaming_event)
    {
        if (libav_input_device::is_open())
        {
            switch(streaming_event)
            {
                case ffmpeg::streaming_event_t::start:
                    change_state(channel_state_t::open);
                break;
                case ffmpeg::streaming_event_t::stop:
                    change_state(channel_state_t::closed);
                break;
                case ffmpeg::streaming_event_t::open:
                    change_state(channel_state_t::connected);
                break;
                case ffmpeg::streaming_event_t::close:
                    change_state(channel_state_t::disconnected);
                break;
                default:;
            }
        }
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
        return device_type_t::v4l2;
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
