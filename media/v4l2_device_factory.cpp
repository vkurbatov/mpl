#include "v4l2_device_factory.h"
#include "v4l2_utils.h"

#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"

#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "v4l2_utils.h"

#include "video_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/v4l2/v4l2_device.h"

#include <shared_mutex>
#include <atomic>
#include <thread>

namespace mpl::media
{

namespace detail
{

}

class v4l2_device : public i_device
{
    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<v4l2_device>;

    struct device_params_t
    {
        device_type_t   device_type = device_type_t::v4l2_in;
        std::string     url;

        device_params_t(device_type_t device_type = device_type_t::v4l2_in
                , const std::string_view& url = {})
            : device_type(device_type)
            , url(url)
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
            return reader.get("url", url);
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("url", url);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::v4l2_in
                    && !url.empty();
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    v4l2::v4l2_device           m_native_device;

    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;
    timestamp_t                 m_real_timestamp;

    timestamp_t                 m_start_time;

    channel_state_t             m_state;
    std::atomic_bool            m_open;

public:

    static u_ptr_t create(const i_property &params)
    {
        device_params_t v4ls_params(params);
        if (v4ls_params.is_valid())
        {
            return std::make_unique<v4l2_device>(v4ls_params);
        }

        return nullptr;
    }

    v4l2_device(const device_params_t& device_params)
        : m_device_params(device_params)
        , m_native_device([&](auto&& frame) { return on_native_frame(std::move(frame)); }
                          , [&](const auto& event) { on_native_device_state(event);} )
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_real_timestamp(0)
        , m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    ~v4l2_device()
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
            reset();

            change_state(channel_state_t::opening);

            v4l2::frame_info_t  format({1280, 720}, 30, v4l2::pixel_format_mjpeg);
            m_native_device.set_format(format);

            if (m_native_device.open(m_device_params.url
                                     , 4))
            {

                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                auto controls = m_native_device.get_control_list();

                v4l2::ctrl_command_t::array_t ctrls;
                for (const auto& c : controls)
                {
                    ctrls.emplace_back(c.id
                                       , 0
                                       , false
                                       , false
                                       , 0);
                }

                auto result = m_native_device.controls(ctrls);

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
        m_real_timestamp = 0;
        m_start_time = mpl::core::utils::now();
    }

    timestamp_t elapsed_time() const
    {
        return mpl::core::utils::now() - m_start_time;
    }

    void process_timesatamp(std::uint32_t fps)
    {
        m_real_timestamp = (elapsed_time() * video_sample_rate) / durations::seconds(1);
        m_frame_timestamp = m_real_timestamp;
        if (fps != 0)
        {
            auto duration = video_sample_rate / fps;
            m_frame_timestamp -= m_frame_timestamp % (duration / 2);
        }
    }

    bool on_native_frame(v4l2::frame_t&& frame)
    {
        video_format_impl video_format(utils::format_form_v4l2(frame.frame_info.pixel_format)
                                       , frame.frame_info.size.width
                                       , frame.frame_info.size.height
                                       , frame.frame_info.fps);

        if (video_format.format_id() != video_format_id_t::undefined
                && !frame.frame_data.empty())
        {
            video_frame_impl video_frame(std::move(video_format)
                                         , m_frame_counter
                                         , m_frame_timestamp
                                         , i_video_frame::frame_type_t::undefined);
            video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                   , smart_buffer(std::move(frame.frame_data)));

            m_frame_counter++;
            process_timesatamp(frame.frame_info.fps);

            message_frame_ref_impl message_frame(video_frame);

            m_router.send_message(message_frame);

        }

        return true;
    }

    void on_native_device_state(const v4l2::streaming_event_t& streaming_event)
    {
        if (v4l2_device::is_open())
        {
            switch(streaming_event)
            {
                case v4l2::streaming_event_t::start:
                    change_state(channel_state_t::open);
                break;
                case v4l2::streaming_event_t::stop:
                    change_state(channel_state_t::closed);
                break;
                case v4l2::streaming_event_t::open:
                    change_state(channel_state_t::connected);
                break;
                case v4l2::streaming_event_t::close:
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
        return device_type_t::v4l2_in;
    }
};

v4l2_device_factory::u_ptr_t v4l2_device_factory::create()
{
    return std::make_unique<v4l2_device_factory>();
}

v4l2_device_factory::v4l2_device_factory()
{

}

i_device::u_ptr_t v4l2_device_factory::create_device(const i_property &device_params)
{
    return v4l2_device::create(device_params);
}

}
