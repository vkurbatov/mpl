#include "ipc_output_device_factory.h"

#include "utils/message_router_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "core/event_channel_state.h"
#include "utils/time_utils.h"
#include "utils/convert_utils.h"
#include "utils/enum_utils.h"
#include "utils/fifo_writer_impl.h"

#include "utils/packetizer.h"

#include "utils/sq/sq_packet_builder.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "tools/base/sync_base.h"

#include <atomic>
#include <thread>
#include <iostream>

namespace mpl::media
{


class wrapped_out_device: public i_message_sink
{
    i_sync_shared_data::s_ptr_t     m_shared_data;
    fifo_writer_impl                m_fifo_writer;
    sq::sq_packet_builder_t         m_sq_builder;
    std::size_t                     m_frame_counter;

    smart_buffer                    m_frame_buffer;

public:

    wrapped_out_device(i_sync_shared_data::s_ptr_t&& shared_data)
        : m_shared_data(std::move(shared_data))
        , m_fifo_writer(*m_shared_data)
        , m_frame_counter(0)
    {

    }

    bool wait(timestamp_t timeout)
    {
        return m_shared_data->wait(timeout);
    }

    std::size_t frames() const
    {
        return m_frame_counter;
    }

    void notify()
    {
        m_shared_data->notify();
    }

    void reset()
    {
        m_frame_counter = 0;
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        switch(message.category())
        {
            case message_category_t::data:
            {
                if (static_cast<const i_message_data&>(message).subclass() == message_class_media
                        && static_cast<const i_message_media_data&>(message).data_type() == media_data_type_t::frame)
                {
                    return send_frame(static_cast<const i_media_frame&>(message));
                }
            }
            break;
            default:;
        }

        return false;
    }

private:
    bool send_frame(const i_media_frame& frame)
    {
        m_frame_counter++;
        m_frame_buffer.clear();
        packetizer packer(m_frame_buffer);

        if (packer.add_enum(message_category_t::data)
                && packer.add_value(frame))
        {

            for (auto&& p : m_sq_builder.build_fragments(m_frame_buffer.data()
                                                         , m_frame_buffer.size()))
            {
                m_fifo_writer.push_data(p.data()
                                        , p.size());
            }

            return true;
        }

        return false;
    }
};

class ipc_output_device : public i_device
{
    using u_ptr_t = std::unique_ptr<ipc_output_device>;

    struct device_params_t
    {
        device_type_t       device_type = device_type_t::ipc_out;
        std::string         channel_name;
        std::size_t         buffer_size;

        device_params_t(device_type_t device_type = device_type_t::ipc_out
                        , const std::string& channel_name = {}
                        , std::size_t buffer_size = 0)
            : device_type(device_type)
            , channel_name(channel_name)
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
            if (reader.get("device_type", device_type_t::ipc_out) == device_type_t::ipc_out)
            {
                return reader.get("channel_name", channel_name)
                        && reader.get("size", buffer_size);
            }
            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::ipc_out)
                    && writer.set("channel_name", channel_name)
                    && writer.set("size", buffer_size);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::ipc_out
                    && !channel_name.empty()
                    && buffer_size > 0;
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    wrapped_out_device          m_wrapped_device;

    channel_state_t             m_state;
    bool                        m_open;

public:

    static u_ptr_t create(i_shared_data_manager& shared_data_manager
                          , const i_property& params)
    {
        device_params_t device_params(params);
        if (device_params.is_valid())
        {
            if (auto shared_data = shared_data_manager.query_data(device_params.channel_name
                                                                  , device_params.buffer_size))
            {
                return std::make_unique<ipc_output_device>(std::move(device_params)
                                                          , std::move(shared_data));
            }
        }

        return nullptr;
    }

    ipc_output_device(device_params_t&& device_params
                      , i_sync_shared_data::s_ptr_t&& shared_data)
        : m_device_params(std::move(device_params))
        , m_wrapped_device(std::move(shared_data))
        , m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    ~ipc_output_device() override
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
            change_state(channel_state_t::opening);
            m_open = true;
            change_state(channel_state_t::open);
            change_state(channel_state_t::connecting);
            change_state(channel_state_t::connected);

            return true;

        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            change_state(channel_state_t::closing);

            m_open = false;

            change_state(channel_state_t::disconnecting);
            change_state(channel_state_t::disconnected);

            change_state(channel_state_t::closed);

            return true;
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
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_wrapped_device;
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
        return device_type_t::ipc_out;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property& input_params) override
    {
        if (!m_open)
        {
            auto device_params = m_device_params;
            if (device_params.load(input_params)
                    && device_params.is_valid())
            {
                m_device_params = device_params;
                return true;
            }
        }

        return false;
    }

    bool get_params(i_property& output_params) const override
    {
        return m_device_params.save(output_params);
    }
};

ipc_output_device_factory::u_ptr_t ipc_output_device_factory::create(i_shared_data_manager &shared_data_manager)
{
    return std::make_unique<ipc_output_device_factory>(shared_data_manager);
}

ipc_output_device_factory::ipc_output_device_factory(i_shared_data_manager &shared_data_manager)
    : m_shared_data_manager(shared_data_manager)
{

}

i_device::u_ptr_t ipc_output_device_factory::create_device(const i_property &device_params)
{
    return ipc_output_device::create(m_shared_data_manager
                                    , device_params);
}



}
