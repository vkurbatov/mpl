#include "v4l2_device_factory.h"
#include "v4l2_utils.h"

#include "core/event_channel_state.h"

#include "utils/message_router_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "utils/time_utils.h"
#include "utils/convert_utils.h"
#include "utils/enum_utils.h"
#include "utils/message_sink_impl.h"
#include "utils/message_command_impl.h"

#include "media/video_frame_impl.h"
#include "media/command_camera_control.h"
#include "media/media_message_types.h"
#include "media/video_frame_impl.h"

#include "v4l2_utils.h"
#include "tools/v4l2/v4l2_utils.h"

#include "tools/utils/sync_base.h"
#include "tools/v4l2/v4l2_input_device.h"

#include <shared_mutex>
#include <condition_variable>
#include <atomic>
#include <thread>

namespace mpl::media
{

constexpr static std::uint32_t control_id_resolution = v4l2::ctrl_format;
constexpr static std::string_view control_name_resolution = "resolution";

namespace detail
{

    std::uint32_t get_ctrl_id(const std::string& ctrl_name)
    {
        if (ctrl_name == control_name_resolution)
        {
            return control_id_resolution;
        }

        return v4l2::get_ctrl_id(ctrl_name);
    }

    std::string get_ctrl_name(std::uint32_t ctrl_id)
    {
        if (ctrl_id == control_id_resolution)
        {
            return std::string(control_name_resolution);
        }

        return v4l2::get_ctrl_name(ctrl_id);
    }

    std::size_t serialize_controls(const v4l2::control_info_t::map_t& controls
                                   , i_property::s_array_t& property_controls)
    {
        for (const auto& c : controls)
        {
            if (auto ctrl = property_helper::create_object())
            {
                property_writer writer(*ctrl);
                const v4l2::control_info_t& control_info = c.second;

                if (writer.set("id", detail::get_ctrl_name(control_info.id))
                        && writer.set("description", control_info.name))
                {
                    switch(control_info.type())
                    {
                        case v4l2::control_type_t::boolean:
                        {
                            if (writer.set("value"
                                           , static_cast<bool>(control_info.current_value)))
                            {
                                property_controls.emplace_back(std::move(ctrl));
                            }

                        }
                        break;
                        case v4l2::control_type_t::numeric:
                        {
                            if (writer.set("value"
                                            , control_info.current_value))
                            {
                                writer.set("min"
                                           , control_info.range.min);
                                writer.set("max"
                                           , control_info.range.max);
                                writer.set("step"
                                           , control_info.step);

                                property_controls.emplace_back(std::move(ctrl));

                            }


                        }
                        break;
                        case v4l2::control_type_t::menu:
                        {
                            if (auto item = control_info.get_menu_item(control_info.current_value))
                            {
                                if (writer.set("value"
                                                , item->name))
                                {

                                    writer.set("menu"
                                               , control_info.menu_list());

                                    property_controls.emplace_back(std::move(ctrl));
                                }


                            }
                        }
                        break;
                        default:;
                    }
                }
            }
        }

        return property_controls.size();
    }
}

class v4l2_device : public i_device
{

    using mutex_t = pt::utils::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;
    using cond_t = std::condition_variable;

    using u_ptr_t = std::unique_ptr<v4l2_device>;

    struct device_params_t
    {
        device_type_t       device_type = device_type_t::v4l2_in;
        std::string         url;
        std::size_t         buffers;

        device_params_t(device_type_t device_type = device_type_t::v4l2_in
                , const std::string_view& url = {}
                , std::size_t buffers = 4)
            : device_type(device_type)
            , url(url)
            , buffers(buffers)
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
            if (reader.get("device_type", device_type_t::v4l2_in) == device_type_t::v4l2_in)
            {
                return reader.get("url", url)
                        | reader.get("buffers", buffers);
            }
            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("device_type", device_type_t::v4l2_in)
                    && writer.set("url", url)
                    && writer.set("buffers", buffers);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::v4l2_in
                    && !url.empty();
        }

        v4l2::v4l2_input_device::config_t native_config() const
        {
            return { url
                    , buffers
                    , 100 };
        }
    };

    class v4l2_wrapper
    {           
        mutable mutex_t                         m_safe_mutex;

        v4l2::v4l2_input_device                 m_native_device;
        mutable v4l2::frame_info_t::array_t     m_cached_formats;
        mutable v4l2::control_info_t::map_t     m_cached_controls;

    public:

        v4l2_wrapper(const device_params_t& params)
            : m_native_device(params.native_config())
        {

        }

        inline bool open()
        {
            if (m_native_device.open())
            {
                m_cached_formats = m_native_device.get_supported_formats();
                get_supported_controls();
                //m_cached_controls = m_native_device.get_supported_controls();

                return true;
            }
            return false;
        }

        inline v4l2::frame_info_t get_format_info() const
        {
            return m_native_device.get_format();
        }

        inline bool close()
        {
            if (m_native_device.close())
            {
                m_cached_controls.clear();
                m_cached_formats.clear();
                return true;
            }

            return false;
        }

        inline bool is_open() const
        {
            return m_native_device.is_opened();
        }

        inline bool set_config(const device_params_t& config)
        {
            return m_native_device.set_config(config.native_config());
        }

        inline device_params_t config() const
        {
            auto native_config = m_native_device.config();
            return { device_type_t::v4l2_in
                        , native_config.url };
        }

        inline bool control(v4l2::ctrl_command_t& command)
        {
            if (command.id == control_id_resolution)
            {
                if (command.is_set)
                {
                    command.success = set_format_id(command.value);
                }
                else if (auto id = get_format_id())
                {
                    command.value = *id;
                    command.success = true;
                }
                else
                {
                    command.success = false;
                }

                return command.success;
            }

            return m_native_device.control(command);
        }

        inline std::size_t controls(v4l2::ctrl_command_t::array_t& commands)
        {
            std::size_t result = 0;

            for (auto& c : commands)
            {
                if (control(c))
                {
                    result++;
                }
            }

            return result;
        }

        inline bool execute_control_command(command_camera_control_t& camera_control)
        {
            if (camera_control.state == command_camera_control_t::state_t::request)
            {
                if (camera_control.commands == nullptr)
                {
                    camera_control.commands = get_controls();
                    return camera_control.commands != nullptr;
                }
                else
                {
                    if (execute_commands(*camera_control.commands) > 0)
                    {
                        return true;
                    }

                }
            }

            return false;
        }

        inline bool set_format_id(std::uint32_t id)
        {
            if (id < m_cached_formats.size())
            {
                return m_native_device.set_format(m_cached_formats[id]);
            }

            return false;
        }

        inline std::string get_format() const
        {
            return m_native_device.get_format().to_string();
        }

        inline std::optional<std::uint32_t> get_format_id() const
        {
            std::uint32_t id = 0;

            auto format = m_native_device.get_format();
            for (const auto& f : m_cached_formats)
            {
                if (f == format)
                {
                    return id;
                }
                id++;
            }

            return std::nullopt;
        }

        inline v4l2::control_info_t get_format_control_info() const
        {
            auto format_id = get_format_id();

            v4l2::control_info_t contorl_info(control_id_resolution
                                              , "Resolution"
                                              , 1
                                              , 0
                                              , format_id.has_value() ? *format_id : 0
                                              , 0
                                              , m_cached_formats.size() > 0 ? m_cached_formats.size() - 1 : 0);


            for (const auto& f : m_cached_formats)
            {
                contorl_info.menu.emplace_back(contorl_info.menu.size(), f.to_string());
            }

            return contorl_info;
        }

        inline v4l2::control_info_t::map_t get_supported_controls(bool cached = false) const
        {
            if (!cached
                    || m_cached_controls.empty())
            {
                m_cached_formats = m_native_device.get_supported_formats();
                m_cached_controls = m_native_device.get_supported_controls();
                m_cached_controls.emplace(control_id_resolution
                                          , get_format_control_info());
            }

            return m_cached_controls;
        }

        const v4l2::control_info_t* get_control(std::uint32_t control_id) const
        {
            if (auto it = m_cached_controls.find(control_id); it != m_cached_controls.end())
            {
                return &it->second;
            }

            return nullptr;
        }

        std::optional<v4l2::ctrl_command_t> create_command(const i_property& command) const
        {
            property_reader reader(command);

            if (auto id_name = reader.get<std::string>("id"))
            {
                bool is_set = reader.has_property("value");

                std::uint32_t id = detail::get_ctrl_id(*id_name);
                if (auto it = m_cached_controls.find(id); it != m_cached_controls.end())
                {
                    const v4l2::control_info_t& control_info = it->second;
                    auto delay = reader.get<std::uint32_t>("delay", 0);
                    if (!is_set)
                    {
                        return v4l2::ctrl_command_t{ control_info.id
                                                  , 0
                                                  , false
                                                  , false
                                                  , delay };
                    }
                    else
                    {
                        switch(control_info.type())
                        {
                            case v4l2::control_type_t::boolean:
                            {
                                if (auto value = reader.get<bool>("value"))
                                {
                                    return v4l2::ctrl_command_t{ control_info.id
                                                              , static_cast<std::int32_t>(*value)
                                                              , true
                                                              , false
                                                              , delay };
                                }
                            }
                            break;
                            case v4l2::control_type_t::numeric:
                            {
                                if (auto value = reader.get<std::int32_t>("value"))
                                {
                                    return v4l2::ctrl_command_t{ control_info.id
                                                              , *value
                                                              , true
                                                              , false
                                                              , delay };
                                }
                            }
                            break;
                            case v4l2::control_type_t::menu:
                            {
                                if (auto value = reader.get<std::string>("value"))
                                {
                                    if (auto item = control_info.get_menu_item(*value))
                                    {
                                        return v4l2::ctrl_command_t{ control_info.id
                                                                      , static_cast<std::int32_t>(item->id)
                                                                      , true
                                                                      , false
                                                                      , delay };
                                    }
                                }
                            }
                            break;
                            default:;
                        }
                    }
                }
            }

            return std::nullopt;
        }

        v4l2::ctrl_command_t::array_t create_commands(const i_property& commands) const
        {
            v4l2::ctrl_command_t::array_t controls;

            if (commands.property_type() == property_type_t::array)
            {
                const auto& ctrls = static_cast<const i_property_array&>(commands).get_value();
                for (const auto& c : ctrls)
                {
                    if (c != nullptr)
                    {
                        if (auto control = create_command(*c))
                        {
                            controls.emplace_back(std::move(*control));
                        }
                    }
                }
            }

            return controls;
        }


        bool execute_command(i_property& output_params)
        {
            if (auto command = create_command(output_params))
            {
                if (control(*command)
                        && command->success)
                {
                    property_writer cmd_writer(output_params);

                    if (command->is_set)
                    {
                        cmd_writer.remove("value");
                        execute_command(output_params);
                        return true;
                    }

                    if (auto control = get_control(command->id))
                    {
                        switch(control->type())
                        {
                            case v4l2::control_type_t::boolean:
                                return cmd_writer.set("value"
                                                      , static_cast<bool>(command->value));
                            break;
                            case v4l2::control_type_t::numeric:
                                return cmd_writer.set("value"
                                                     , command->value);
                            break;
                            case v4l2::control_type_t::menu:
                                if (auto item = control->get_menu_item(command->value))
                                {
                                    return cmd_writer.set("value"
                                                          , item->name);
                                }
                            break;
                            default:;
                        }
                    }
                }
            }

            return false;
        }


        inline std::size_t execute_commands(i_property& output_params)
        {
            std::size_t result = 0;

            if (output_params.property_type() == property_type_t::array)
            {
                const auto& ctrls = static_cast<const i_property_array&>(output_params).get_value();
                for (const auto& c : ctrls)
                {
                    if (c != nullptr)
                    {
                        if (execute_command(*c))
                        {
                            result++;
                        }
                    }
                }
            }

            return result;
        }

        inline i_property::u_ptr_t get_controls() const
        {
            if (auto ctrls = property_helper::create_array())
            {
                auto& control_array = static_cast<i_property_array&>(*ctrls).get_value();
                detail::serialize_controls(get_supported_controls(false)
                                           , control_array);



                return ctrls;
            }

            return nullptr;
        }

        inline bool read_frame(v4l2::frame_t& frame)
        {
            return m_native_device.read_frame(frame);
        }
    };

    using commands_queue_t = std::queue<command_camera_control_t>;

    mutable mutex_t             m_command_mutex;
    cond_t                      m_command_signal;
    device_params_t             m_device_params;
    v4l2_wrapper                m_wrapped_device;

    message_sink_impl           m_sink;
    message_router_impl         m_router;


    commands_queue_t            m_commands;
    frame_id_t                  m_frame_counter;
    timestamp_t                 m_frame_timestamp;
    timestamp_t                 m_real_timestamp;

    timestamp_t                 m_start_time;

    std::thread                 m_thread;

    channel_state_t             m_state;
    std::atomic_bool            m_running;
    bool                        m_open;

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
        , m_wrapped_device(m_device_params)
        , m_sink([&](const auto& message) { return on_message(message); })
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_real_timestamp(0)
        , m_state(channel_state_t::ready)
        , m_running(false)
        , m_open(false)
    {

    }

    ~v4l2_device() override
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
            m_running.store(true, std::memory_order_release);

            change_state(channel_state_t::opening);

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
            m_command_signal.notify_all();

            if (m_thread.joinable())
            {
                m_thread.join();
            }

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
        m_start_time = utils::time::now();
    }

    timestamp_t elapsed_time() const
    {
        return utils::time::now() - m_start_time;
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

    bool on_native_frame(v4l2::frame_t& frame)
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
            video_frame.smart_buffers().set_buffer(media_buffer_index
                                                   , smart_buffer(std::move(frame.frame_data)));

            m_frame_counter++;
            process_timesatamp(frame.frame_info.fps);

            m_router.send_message(video_frame);

        }

        return true;
    }

    bool on_message(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::command:
            {
                auto& command_message = static_cast<const i_message_command&>(message);
                switch(command_message.command().command_id)
                {
                    case command_camera_control_t::id:
                    {
                        return on_camera_control(static_cast<const command_camera_control_t&>(command_message.command()));
                    }
                    break;
                    default:;
                }
            }
            break;
            default:;
        }

        return false;
    }

    bool on_camera_control(const command_camera_control_t& camera_control)
    {
        if (is_open())
        {
            lock_t lock(m_command_mutex);
            m_commands.push(camera_control);
            m_command_signal.notify_all();

            return true;
        }
        return false;
    }

    bool fetch_control_command(command_camera_control_t& camera_control)
    {
        lock_t lock(m_command_mutex);
        if (!m_commands.empty())
        {
            camera_control = std::move(m_commands.front());
            m_commands.pop();
            return true;
        }

        return false;
    }

    void grabbing_thread()
    {
        std::mutex signal_mutex;
        std::unique_lock signal_lock(signal_mutex);
        change_state(channel_state_t::open);

        std::size_t error_counter = 0;
        std::uint32_t frame_time = 1000; //(1000 / 60) - 1;

        while(is_running())
        {
            change_state(channel_state_t::connecting);          
            if (m_wrapped_device.open())
            {
                frame_time = 100;

                change_state(channel_state_t::connected);

                error_counter = 0;

                while (is_running()
                       && error_counter < 10)
                {
                    command_camera_control_t camera_control;
                    if (fetch_control_command(camera_control))
                    {
                        if (m_wrapped_device.execute_control_command(camera_control))
                        {
                            camera_control.state = command_camera_control_t::state_t::success;
                            error_counter = 0;
                        }
                        else
                        {
                            camera_control.state = command_camera_control_t::state_t::failed;
                        }
                        m_router.send_message(message_command_impl<command_camera_control_t, message_class_media>(camera_control));
                        continue;
                    }

                    v4l2::frame_t v4l2_frame;
                    if (m_wrapped_device.read_frame(v4l2_frame))
                    {
                        error_counter = 0;
                        on_native_frame(v4l2_frame);
                        if (v4l2_frame.frame_info.fps != 0)
                        {
                            frame_time = 1000 / v4l2_frame.frame_info.fps;
                        }
                        continue;
                    }
                    else
                    {
                        error_counter++;
                    }

                    if (is_running())
                    {
                        m_command_signal.wait_for(signal_lock, std::chrono::milliseconds(frame_time));
                    }
                }

                change_state(channel_state_t::disconnecting);
                m_wrapped_device.close();
                change_state(channel_state_t::disconnected);
            }
        }
    }


    bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
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

    // i_device interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_sink;
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

    device_type_t device_type() const override
    {
        return device_type_t::v4l2_in;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property& input_params) override
    {
        bool result = false;
        auto device_params = m_device_params;
        if (device_params.load(input_params))
        {
            if (!m_wrapped_device.is_open())
            {
                m_device_params = device_params;
                result = true;
            }
        }

        return result;
    }

    bool get_params(i_property& output_params) const override
    {
        return m_device_params.save(output_params);
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
