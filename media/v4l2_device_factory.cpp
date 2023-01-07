#include "v4l2_device_factory.h"
#include "message_router_impl.h"
#include "video_frame_impl.h"
#include "property_writer.h"

#include "message_event_impl.h"
#include "event_channel_state.h"
#include "video_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/v4l2/v4l2_device.h"

#include <shared_mutex>
#include <atomic>

namespace mpl
{

namespace detail
{

video_format_id_t format_from_v4l2(v4l2::pixel_format_t pixel_format)
{
    return video_format_id_t::undefined;
}

}

class v4l2_device : public i_device
{
    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<v4l2_device>;

    struct device_params_t
    {
        device_type_t   device_type;
        std::string     url;
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    v4l2::v4l2_device           m_native_device;

    channel_state_t             m_state;
    std::atomic_bool            m_open;

public:

    static u_ptr_t create(const i_property &device_params)
    {
        return nullptr;
    }

    v4l2_device()
        : m_native_device([&](auto&& frame) { return on_native_frame(std::move(frame)); }
                          , [&](const auto& event) { on_native_device_state(event);} )
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
                                           , false
                                           , std::memory_order_acquire))
        {
            change_state(channel_state_t::opening);
            if (m_native_device.open(m_device_params.url
                                     , 2))
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

            return true;

        }

        return false;
    }

    bool on_native_frame(v4l2::frame_t&& frame)
    {

        return false;
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
        return device_type_t::v4l2;
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
