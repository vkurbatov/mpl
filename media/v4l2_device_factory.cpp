#include "v4l2_device_factory.h"
#include "v4l2_utils.h"

#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"
#include "core/convert_utils.h"
#include "core/enum_utils.h"
#include "core/message_sink_impl.h"

#include "video_frame_impl.h"
#include "message_frame_impl.h"
#include "media_command_message_impl.h"
#include "command_camera_control.h"

#include "v4l2_utils.h"
#include "tools/v4l2/v4l2_utils.h"

#include "video_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/v4l2/v4l2_input_device.h"

#include <shared_mutex>
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
                                   , i_property::array_t& property_controls)
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

    using mutex_t = base::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    using u_ptr_t = std::unique_ptr<v4l2_device>;

    struct device_params_t
    {
        device_type_t       device_type = device_type_t::v4l2_in;
        std::string         url;
        std::size_t         buffers;

        device_params_t(device_type_t device_type = device_type_t::v4l2_in
                , const std::string_view& url = {}
                , std::size_t buffers = 10)
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
                    , 50 };
        }
    };

    class v4l2_wrapper
    {           
        mutable mutex_t                         m_safe_mutex;

        v4l2::v4l2_input_device                 m_native_device;
        v4l2::frame_info_t                      m_frame_info;
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
                command.success = set_format_id(command.value);
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

        inline bool execute_command(command_camera_control_t& camera_control)
        {
            switch (camera_control.state)
            {
                case command_camera_control_t::state_t::set:
                {
                    if (camera_control.commands != nullptr)
                    {
                        if (send_commands(*camera_control.commands))
                        {
                            recv_commands(*camera_control.commands);
                            camera_control.state = command_camera_control_t::state_t::ok;
                            return true;
                        }
                    }
                }
                break;
                case command_camera_control_t::state_t::get:
                {
                    if (camera_control.commands == nullptr)
                    {
                        camera_control.commands = get_controls();
                        camera_control.state = command_camera_control_t::state_t::ok;
                        return true;

                    }
                    else
                    {
                        if (recv_commands(*camera_control.commands))
                        {
                            camera_control.state = command_camera_control_t::state_t::ok;
                            return true;
                        }
                    }
                }
                break;
                default:;
            }

            camera_control.state = command_camera_control_t::state_t::failed;
            return false;
        }

        inline bool set_format(const std::string& format_string)
        {
            if (!format_string.empty())
            {
                for (const auto& f : m_cached_formats)
                {
                    if (f.to_string() == format_string)
                    {
                        return m_native_device.set_format(f);
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

        inline uint32_t get_format_id() const
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

            return 0;
        }

        std::vector<std::string> get_supported_formats(bool cached = false) const
        {
            std::vector<std::string> formats;

            v4l2::frame_info_t::array_t v4l2_formats;
            if (!cached || m_cached_formats.empty())
            {
                v4l2_formats = m_native_device.get_supported_formats();
                m_cached_formats = v4l2_formats;
            }
            else
            {
                v4l2_formats = m_cached_formats;
            }

            for (const auto& f : v4l2_formats)
            {
                auto format_string = f.to_string();
                if (!format_string.empty())
                {
                    formats.emplace_back(std::move(format_string));
                }
            }

            return formats;
        }

        inline v4l2::control_info_t get_format_control_info(bool cached = false) const
        {
            auto formats = get_supported_formats(cached);

            v4l2::control_info_t contorl_info(control_id_resolution
                                              , "Resolution"
                                              , 1
                                              , 0
                                              , get_format_id()
                                              , 0
                                              , formats.size() > 0 ? formats.size() - 1 : 0);


            for (const auto& d : formats)
            {
                contorl_info.menu.emplace_back(contorl_info.menu.size(), d);
            }

            return contorl_info;
        }

        inline v4l2::control_info_t::map_t get_supported_controls(bool cached = false) const
        {
            if (!cached
                    || m_cached_controls.empty())
            {
                m_cached_controls = m_native_device.get_supported_controls();
                m_cached_controls.emplace(control_id_resolution
                                          , get_format_control_info(false));
            }
            return m_cached_controls;
        }

        inline bool get_formats(i_property& params)
        {
            auto formats = get_supported_formats();
            return !formats.empty()
                    && property_writer(params).set({}, formats);
        }

        i_property::u_ptr_t get_formats()
        {
            if (auto formats = property_helper::create_array())
            {
                if (get_formats(*formats))
                {
                    return formats;
                }
            }

            return nullptr;
        }

        std::optional<v4l2::ctrl_command_t> create_command(const i_property& command
                                                           , bool is_set) const
        {
            property_reader reader(command);

            if (auto id_name = reader.get<std::string>("id"))
            {
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

        v4l2::ctrl_command_t::array_t create_commands(const i_property& commands
                                                      , bool is_set) const
        {
            v4l2::ctrl_command_t::array_t controls;

            if (commands.property_type() == property_type_t::array)
            {
                const auto& ctrls = static_cast<const i_property_array&>(commands).get_value();
                for (const auto& c : ctrls)
                {
                    if (c != nullptr)
                    {

                        if (auto control = create_command(*c
                                                            , is_set))
                        {
                            controls.emplace_back(std::move(*control));
                        }
                    }
                }
            }

            return controls;
        }


        inline bool send_commands(const i_property& input_params)
        {
            auto send_controls = create_commands(input_params
                                            , true);
            return controls(send_controls) > 0;

        }

        bool recv_commands(i_property& output_params)
        {
            bool result = false;

            if (output_params.property_type() == property_type_t::array)
            {
                for (auto& c : static_cast<i_property_array&>(output_params).get_value())
                {
                    if (auto command = create_command(*c
                                                      , false))
                    {
                        if (control(*command))
                        {
                            if (command->success)
                            {
                                auto it = m_cached_controls.find(command->id);
                                if (it != m_cached_controls.end())
                                {
                                    const v4l2::control_info_t& control_info = it->second;
                                    property_writer cmd_writer(*c);

                                    switch(control_info.type())
                                    {
                                        case v4l2::control_type_t::boolean:
                                            result |= cmd_writer.set("value"
                                                                    , static_cast<bool>(command->value));
                                        break;
                                        case v4l2::control_type_t::numeric:
                                            result |= cmd_writer.set("value"
                                                                    , command->value);
                                        break;
                                        case v4l2::control_type_t::menu:
                                            if (auto item = control_info.get_menu_item(command->value))
                                            {

                                                result |= cmd_writer.set("value"
                                                                        , item->name);
                                            }
                                        break;
                                        default:;
                                    }
                                }
                            }
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
                detail::serialize_controls(get_supported_controls()
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

            message_frame_ref_impl message_frame(video_frame);

            m_router.send_message(message_frame);

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

            return true;
        }
        return false;
    }

    bool fetch_camera_control(command_camera_control_t& camera_control)
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
        change_state(channel_state_t::open);

        std::size_t error_counter = 0;
        std::uint32_t frame_time = 1000; //(1000 / 60) - 1;

        while(is_running())
        {
            change_state(channel_state_t::connecting);
            if (m_wrapped_device.open())
            {
                change_state(channel_state_t::connected);

                error_counter = 0;

                while (is_running()
                       && error_counter < 10)
                {
                    command_camera_control_t camera_control;
                    if (fetch_camera_control(camera_control))
                    {
                        m_wrapped_device.execute_command(camera_control);
                        m_router.send_message(media_command_message_impl<command_camera_control_t>(camera_control));
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
                    }
                    else
                    {
                        error_counter++;
                    }

                    if (is_running())
                    {
                        core::utils::sleep(durations::milliseconds(frame_time));
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
