#include "v4l2_device_factory.h"
#include "message_router_impl.h"
#include "video_frame_impl.h"
#include "property_writer.h"

#include <atomic>

namespace mpl
{

class v4l2_device : public i_device
{
    using u_ptr_t = std::unique_ptr<v4l2_device>;

    struct device_params_t
    {
        device_type_t   device_type;
        std::string     url;


    };

    message_router_impl         m_router;

    channel_state_t             m_state;
    std::atomic_bool            m_open;

    static u_ptr_t create(const i_property &device_params)
    {
        return nullptr;
    }

    v4l2_device()
        : m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    bool open()
    {
        bool expected = false;
        if (m_open.compare_exchange_strong(expected
                                           , true
                                           , std::memory_order_acquire))
        {

        }

        return false;
    }

    bool close()
    {
        bool expected = true;
        if (m_open.compare_exchange_strong(expected
                                           , false
                                           , std::memory_order_acquire))
        {

        }

        return false;
    }

    // i_channel interface
public:
    bool control(const channel_control_t &control) override
    {
        switch(control.control_id)
        {
            case channel_control_id_t::open:

            break;
            case channel_control_id_t::close:

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
    return nullptr;
}

}
