#include "ipc_input_device_factory.h"

#include "utils/message_router_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "core/event_channel_state.h"
#include "utils/time_utils.h"
#include "utils/convert_utils.h"
#include "utils/enum_utils.h"
#include "utils/time_utils.h"
#include "utils/fifo_reader_impl.h"

#include "utils/depacketizer.h"

#include "utils/sq/sq_parser.h"
#include "utils/sq/sq_stitcher.h"

#include "audio_frame_impl.h"
#include "video_frame_impl.h"
#include "message_frame_impl.h"


#include <shared_mutex>
#include <atomic>
#include <thread>
#include <iostream>


namespace mpl::media
{

class wrapped_in_device
{
    static constexpr std::size_t default_recv_buffer_size = 1024 * 1024;

    i_sync_shared_data::s_ptr_t     m_shared_data;
    i_message_sink&                 m_message_sink;
    fifo_reader_impl                m_fifo_reader;
    sq::sq_parser                   m_sq_parser;
    sq::sq_stitcher                 m_sq_stitcher;
    std::size_t                     m_frame_counter;

    raw_array_t                     m_recv_buffer;

public:

    wrapped_in_device(i_sync_shared_data::s_ptr_t&& shared_data
                   , i_message_sink& message_sink)
        : m_shared_data(std::move(shared_data))
        , m_message_sink(message_sink)
        , m_fifo_reader(*m_shared_data)
        , m_sq_parser([&](sq::sq_packet&& packet) { on_sq_packet(std::move(packet)); })
        , m_sq_stitcher([&](smart_buffer&& frame) { on_frame(std::move(frame)); }
                        , 4)
        , m_frame_counter(0)
        , m_recv_buffer(default_recv_buffer_size)
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

    std::size_t read_data()
    {
        std::size_t result = 0;

        while(auto size = m_fifo_reader.pop_data(m_recv_buffer.data()
                                                 , m_recv_buffer.size()))
        {
            if (size == i_fifo_buffer::overload)
            {
                reset();
                break;
            }

            m_sq_parser.push_stream(m_recv_buffer.data()
                                    , size);
            result += size;

            if (size < m_recv_buffer.size())
            {
                break;
            }
        }


        return result;
    }

    void notify()
    {
        m_shared_data->notify();
    }

    void reset()
    {
        m_frame_counter = 0;
        m_fifo_reader.reset();
        m_sq_parser.reset();
        m_sq_stitcher.reset();
    }

private:

    void on_sq_packet(sq::sq_packet&& packet)
    {
        if (packet.is_valid())
        {
            m_sq_stitcher.push_packet(std::move(packet));
        }
    }

    void on_frame(smart_buffer&& buffer)
    {
        depacketizer depacker(buffer);
        message_category_t category = message_category_t::undefined;

        if (depacker.fetch_enum(category)
                && category == message_category_t::frame)
        {
            auto save = depacker.cursor();
            media_type_t media_type = media_type_t::undefined;
            if (depacker.open_object()
                    && depacker.open_object()
                    && depacker.fetch_enum(media_type))
            {
                depacker.seek(save);
                switch(media_type)
                {
                    case media_type_t::audio:
                    {
                        audio_frame_impl audio_frame({});
                        if (depacker.fetch_value(audio_frame))
                        {
                            message_frame_ref_impl message_frame(audio_frame);

                            m_frame_counter++;
                            m_message_sink.send_message(message_frame);
                        }
                    }
                    break;
                    case media_type_t::video:
                    {
                        video_frame_impl video_frame({});
                        if (depacker.fetch_value(video_frame))
                        {
                            message_frame_ref_impl message_frame(video_frame);

                            m_frame_counter++;
                            m_message_sink.send_message(message_frame);
                        }
                    }
                    break;
                    default:;
                }
            }
        }
    }

};

class ipc_input_device : public i_device
{
    using u_ptr_t = std::unique_ptr<ipc_input_device>;

    struct device_params_t
    {
        device_type_t       device_type = device_type_t::ipc_in;
        std::string         channel_name;

        device_params_t(device_type_t device_type = device_type_t::ipc_in
                        , const std::string& channel_name = {})
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
            if (reader.get("device_type", device_type_t::ipc_in) == device_type_t::ipc_in)
            {
                return reader.get("channel_name", channel_name);
            }
            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::ipc_in)
                    && writer.set("channel_name", channel_name);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::ipc_in
                    && !channel_name.empty();
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    wrapped_in_device           m_wrapped_device;

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
            if (auto shared_data = shared_data_manager.query_data(device_params.channel_name
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
        , m_wrapped_device(std::move(shared_data)
                           , m_router)
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
            change_state(channel_state_t::opening);
            m_open = true;
            m_running.store(true, std::memory_order_release);


            m_thread = std::thread([&]{ grabbing_thread(); });
            return true;

        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {
            change_state(channel_state_t::closing);

            m_running.store(false, std::memory_order_release);
            m_open = false;
            m_wrapped_device.notify();

            if (m_thread.joinable())
            {
                m_thread.join();
            }

            change_state(channel_state_t::closed);

            return true;
        }

        return false;
    }

    bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
    }


    void grabbing_thread()
    {
        change_state(channel_state_t::open);

        while(is_running())
        {
            m_wrapped_device.reset();
            change_state(channel_state_t::connecting);

            bool connected = false;

            do
            {
                m_wrapped_device.read_data();
                if (connected == false
                        && m_wrapped_device.frames() > 0)
                {
                    change_state(channel_state_t::connected);
                }
                mpl::core::utils::sleep(durations::milliseconds(50));
            }
            while(is_running());

            if (connected)
            {
                change_state(channel_state_t::disconnecting);
                change_state(channel_state_t::disconnected);
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
        return device_type_t::ipc_in;
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
