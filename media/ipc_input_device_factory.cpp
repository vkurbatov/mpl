#include "ipc_input_device_factory.h"

#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"
#include "core/convert_utils.h"
#include "core/enum_utils.h"
#include "core/ipc/ipc_manager_impl.h"

#include "core/packetizer.h"
#include "core/depacketizer.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"
#include "message_frame_impl.h"


#include <shared_mutex>
#include <atomic>
#include <thread>


namespace mpl::media
{

class ipc_input_device : public i_device
{
    using u_ptr_t = std::unique_ptr<ipc_input_device>;

    struct device_params_t
    {
        device_type_t       device_type = device_type_t::ipc_in;
        std::string         input_channel_name;

        device_params_t(device_type_t device_type = device_type_t::ipc_in
                        , const std::string& input_channel_name = {})
            : device_type(device_type)
            , input_channel_name(input_channel_name)
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
            if (reader.get("device_type", device_type_t::ipc_in) == device_type_t::ipc_in)
            {
                return reader.get("input_channel_name", input_channel_name);
            }
            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::ipc_in)
                    && writer.set("input_channel_name", input_channel_name);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::ipc_in
                    && !input_channel_name.empty();
        }
    };

    device_params_t             m_device_params;
    i_shared_data::s_ptr_t      m_shared_data;
    message_router_impl         m_router;

    std::thread                 m_thread;

    channel_state_t             m_state;
    std::atomic_bool            m_running;
    bool                        m_open;

public:

    static u_ptr_t create(i_shared_data_manager& shared_data_manager
                          , const i_property& params)
    {
        device_params_t device_params(params);
        if (device_params.is_valid())
        {
            if (auto shared_data = shared_data_manager.query_data(device_params.input_channel_name
                                                                  , 0))
            {
                return std::make_unique<ipc_input_device>(std::move(device_params)
                                                          , std::move(shared_data));
            }
        }

        return nullptr;
    }

    ipc_input_device(device_params_t&& device_params
                     , i_sync_shared_data::s_ptr_t&& shared_data)
        : m_device_params(std::move(device_params))
        , m_shared_data(std::move(shared_data))
        , m_state(channel_state_t::ready)
        , m_running(false)
        , m_open(false)
    {

    }

    ~ipc_input_device() override
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
        if (!m_open)
        {
            m_open = true;

        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            m_open = false;

            return true;
        }

        return false;
    }

    bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
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
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink() override
    {
        return nullptr;
    }
    i_message_source *source() override
    {
        return &m_router;
    }

    // i_device interface
public:
    device_type_t device_type() const override
    {
        return device_type_t::ipc_in;
    }
};

ipc_input_device_factory::u_ptr_t ipc_input_device_factory::create(i_shared_data_manager& shared_data_manager)
{
    return std::make_unique<ipc_input_device_factory>(shared_data_manager);
}

ipc_input_device_factory::ipc_input_device_factory(i_shared_data_manager& shared_data_manager)
    : m_shared_data_manager(shared_data_manager)
{

}

i_device::u_ptr_t ipc_input_device_factory::create_device(const i_property &device_params)
{
    return ipc_input_device::create(m_shared_data_manager
                                    , device_params);
}



}
