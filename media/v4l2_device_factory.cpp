#include "v4l2_device_factory.h"
#include "v4l2_utils.h"

#include "core/message_router_impl.h"
#include "core/property_writer.h"
#include "core/message_event_impl.h"
#include "core/event_channel_state.h"
#include "core/time_utils.h"
#include "core/convert_utils.h"
#include "core/enum_utils.h"

#include "video_frame_impl.h"
#include "message_frame_impl.h"

#include "v4l2_utils.h"

#include "video_frame_impl.h"

#include "tools/base/sync_base.h"
#include "tools/v4l2/v4l2_input_device.h"

#include <shared_mutex>
#include <atomic>
#include <thread>

namespace mpl::media
{

namespace detail
{

    std::size_t serialize_controls(const v4l2::control_info_t::map_t& controls
                                   , i_property::array_t& property_controls)
    {
        for (const auto& c : controls)
        {
            if (auto ctrl = property_helper::create_object())
            {
                property_writer writer(*ctrl);
                const v4l2::control_info_t& control_info = c.second;

                if (writer.set("id", control_info.id)
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

    std::string get_format_string(const v4l2::frame_info_t& frame_info)
    {
        std::string result;

        video_format_id_t format_id = utils::format_form_v4l2(frame_info.pixel_format);
        if (format_id != video_format_id_t::undefined)
        {
            result.append(std::to_string(frame_info.size.width)).append("x")
                    .append(std::to_string(frame_info.size.height)).append("@")
                    .append(std::to_string(frame_info.fps)).append(":")
                    .append(core::utils::enum_to_string(format_id));
        }

        return result;
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
        mutable v4l2::control_info_t::map_t     m_cached_controls;
        mutable v4l2::frame_info_t::array_t     m_cached_formats;

    public:

        v4l2_wrapper(const device_params_t& params)
            : m_native_device(params.native_config())
        {

        }

        inline void lock()
        {
            m_safe_mutex.lock();
        }

        inline void unlock()
        {
            m_safe_mutex.unlock();
        }

        inline bool open()
        {
            lock_t lock(m_safe_mutex);
            {
                if (m_native_device.open())
                {
                    m_cached_controls = m_native_device.get_supported_controls();
                    m_cached_formats = m_native_device.get_supported_formats();
                    return true;
                }
            }
            return false;
        }

        inline bool close()
        {
            lock_t lock(m_safe_mutex);
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
            shared_lock_t lock(m_safe_mutex);
            return m_native_device.is_opened();
        }

        inline bool set_config(const device_params_t& config)
        {
            lock_t lock(m_safe_mutex);
            return m_native_device.set_config(config.native_config());
        }

        inline device_params_t config() const
        {
            shared_lock_t lock(m_safe_mutex);
            auto native_config = m_native_device.config();
            return { device_type_t::v4l2_in
                        , native_config.url };
        }

        inline std::size_t controls(v4l2::ctrl_command_t::array_t& controls)
        {
            lock_t lock(m_safe_mutex);
            return m_native_device.controls(controls);
        }

        inline bool control(v4l2::ctrl_command_t& control)
        {
            lock_t lock(m_safe_mutex);
            return m_native_device.control(control);
        }

        inline bool set_format(const i_video_format& format)
        {
            v4l2::frame_info_t frame_info;
            if (mpl::core::utils::convert(format
                                          , frame_info))
            {
                lock_t lock(m_safe_mutex);
                return m_native_device.set_format(frame_info);
            }

            return false;
        }

        bool get_format(video_format_impl& format) const
        {
            v4l2::frame_info_t frame_info;
            {
                shared_lock_t lock(m_safe_mutex);
                frame_info = m_native_device.get_format();
            }

            if (!frame_info.is_null())
            {
                return mpl::core::utils::convert(frame_info
                                                 , format);
            }

            return false;

        }

        std::vector<video_format_impl> get_supported_formats(bool cached = false) const
        {
            std::vector<video_format_impl> formats;
            v4l2::frame_info_t::array_t v4l2_formats;
            {
                shared_lock_t lock(m_safe_mutex);
                if (!cached || m_cached_formats.empty())
                {
                    v4l2_formats = m_native_device.get_supported_formats();
                    m_cached_formats = v4l2_formats;
                }
                else
                {
                    v4l2_formats = m_cached_formats;
                }
            }

            for (const auto& f : v4l2_formats)
            {
                video_format_impl format;
                if (mpl::core::utils::convert(f
                                              , format))
                {
                    formats.emplace_back(std::move(format));
                }
            }

            return formats;
        }

        v4l2::control_info_t::map_t get_supported_controls(bool cached = false) const
        {
            shared_lock_t lock(m_safe_mutex);
            if (!cached
                    || m_cached_controls.empty())
            {
                m_cached_controls = m_native_device.get_supported_controls();
            }
            return m_cached_controls;
        }

        bool get_formats(i_property& params)
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

            if (auto id = reader.get<std::uint32_t>("id"))
            {
                if (auto it = m_cached_controls.find(*id); it != m_cached_controls.end())
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

        bool send_commands(const i_property& input_params)
        {
            bool result = false;
            property_reader reader(input_params);
            if (auto commands = reader["commands"])
            {
                auto controls = create_commands(*commands
                                                , true);

                result |= m_native_device.controls(controls) > 0;
            }

            if (auto format = reader.get<video_format_impl>("format"))
            {
                v4l2::frame_info_t frame_info;
                if (core::utils::convert<i_video_format>(*format
                                                         , frame_info))
                {
                    result |= m_native_device.set_format(frame_info);
                }
            }

            return result;
        }

        bool recv_commands(i_property& output_params)
        {
            bool result = false;
            property_writer writer(output_params);

            if (auto commands = writer["commands"])
            {
                if (commands->property_type() == property_type_t::array)
                {
                    for (auto& c : static_cast<i_property_array&>(*commands).get_value())
                    {
                        if (auto command = create_command(*c
                                                          , false))
                        {
                            if (m_native_device.control(*command))
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
            }

            if (writer.has_property("format"))
            {
                video_format_impl video_format;
                if (core::utils::convert(m_native_device.get_format()
                                         , video_format))
                {
                    result |= writer.set("format"
                                         , video_format);
                }
            }

            return result;
        }

        bool command(const i_property* input_params
                     , i_property* output_params)
        {
            lock_t lock(m_safe_mutex);
            if (m_native_device.is_opened())
            {
                bool result = false;
                if (input_params)
                {
                    result = send_commands(*input_params);
                }

                if (output_params)
                {
                    result = recv_commands(*output_params);
                }

                return result;
            }

            return false;
        }

        inline i_property::u_ptr_t get_controls()
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
            lock_t lock(m_safe_mutex);
            return m_native_device.read_frame(frame);
        }
    };

    device_params_t             m_device_params;
    message_router_impl         m_router;
    v4l2_wrapper                m_wrapped_device;

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
            video_frame.smart_buffers().set_buffer(main_media_buffer_index
                                                   , smart_buffer(std::move(frame.frame_data)));

            m_frame_counter++;
            process_timesatamp(frame.frame_info.fps);

            message_frame_ref_impl message_frame(video_frame);

            return m_router.send_message(message_frame);

        }

        return true;
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

    bool set_params(const i_property& input_params)
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
/*
        property_reader reader(input_params);
        if (auto params = reader["controls"])
        {

        }*/

        return result;
    }

    bool get_params(i_property& output_params)
    {
        if (m_device_params.save(output_params))
        {
            if (m_wrapped_device.is_open())
            {
                property_writer writer(output_params);
                /*if (auto formats = m_wrapped_device.get_formats())
                {
                    writer.set("formats", *formats);
                }*/
                if (auto controls = m_wrapped_device.get_controls())
                {
                    writer.set("controls", *controls);
                }
            }
            return true;
        }

        return false;
    }

    bool internal_configure(const i_property* input_params
                            , i_property* output_params)
    {
        bool result = false;

        if (input_params != nullptr)
        {
            result = set_params(*input_params);
        }

        if (output_params != nullptr)
        {
            result = get_params(*output_params);
        }

        return result;
    }

    bool internal_command(const i_property* input_params
                          , i_property* output_params)
    {
        if (input_params != nullptr
                || output_params != nullptr)
        {
            return m_wrapped_device.command(input_params
                                            , output_params);
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
            case channel_control_id_t::configure:
                return internal_configure(control.input_params
                                          , control.output_params);
            break;
            case channel_control_id_t::command:
                return internal_command(control.input_params
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
