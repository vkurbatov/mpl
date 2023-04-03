#include "v4l2_input_device.h"
#include "v4l2_api.h"

#include <thread>

namespace v4l2
{

struct v4l2_object_t
{
    using u_ptr_t = std::unique_ptr<v4l2_object_t>;

    std::int32_t    handle;
    mapped_buffer_t mapped_buffer;

    static u_ptr_t create(const std::string& url
                          , std::size_t buffer_count
                          , const frame_info_t& frame_info = {})
    {
        if (auto dev = std::make_unique<v4l2_object_t>(url
                                                       , buffer_count
                                                       , frame_info))
        {
            if (dev->is_open())
            {
                return dev;
            }
        }

        return nullptr;
    }

    v4l2_object_t(const std::string& url
                  , std::size_t buffer_count
                  , const frame_info_t& frame_info)

        : handle(v4l2::open_device(url))
    {
        if (handle >= 0)
        {
            if (!frame_info.is_null())
            {
                set_frame_info(frame_info);
            }
            mapped_buffer = v4l2::map(handle
                                      , buffer_count);
        }
    }

    ~v4l2_object_t()
    {
        if (handle >= 0)
        {

            v4l2::unmap(handle
                        , mapped_buffer);
            v4l2::close_device(handle);
        }
    }


    bool fetch_frame_format(frame_size_t& frame_size
                            , pixel_format_t& pixel_format)
    {
        return v4l2::fetch_frame_format(handle
                                        , frame_size
                                        , pixel_format);
    }

    bool fetch_fps(std::uint32_t& fps)
    {
        return v4l2::fetch_fps(handle
                               , fps);
    }

    bool fetch_frame_info(frame_info_t& frame_info)
    {
        return fetch_frame_format(frame_info.size
                                  , frame_info.pixel_format)
                && fetch_fps(frame_info.fps);
    }

    frame_info_t::array_t fetch_supported_format_list()
    {
        return v4l2::fetch_supported_format(handle);
    }

    bool set_frame_format(const frame_size_t& frame_size
                          , pixel_format_t pixel_format)
    {

        return v4l2::set_frame_format(handle
                                      , frame_size
                                      , pixel_format);
    }

    bool set_fps(std::uint32_t fps)
    {
        return v4l2::set_fps(handle
                             , fps);
    }

    bool set_frame_info(const frame_info_t& frame_info)
    {

        if (handle >= 0)
        {

            return set_frame_format(frame_info.size
                                    , frame_info.pixel_format)
                        && set_fps(frame_info.fps);
        }

        return false;

    }

    control_info_t::map_t fetch_control_list()
    {
        return v4l2::fetch_control_list(handle);
    }

    frame_data_t fetch_frame_data(std::uint32_t timeout = 0)
    {
        return v4l2::fetch_frame_data(handle
                                      , mapped_buffer
                                      , timeout);
    }

    bool is_open() const
    {
        return handle >= 0;
    }

    bool set_control(std::uint32_t control_id
                     , std::int32_t value)
    {
        return v4l2::set_control(handle
                                 , control_id
                                 , value);
    }

    bool get_control(std::uint32_t control_id
                     , std::int32_t& value)
    {
        return v4l2::get_control(handle
                                 , control_id
                                 , value);
    }

    bool control(ctrl_command_t& command)
    {
        command.success = command.is_set
                ? set_control(command.id, command.value)
                : get_control(command.id, command.value);

        if (command.success
                && command.delay_ms > 0)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(command.delay_ms));
        }

        return command.success;
    }

};

struct v4l2_input_device::context_t
{
    using config_t = v4l2_input_device::config_t;
    using u_ptr_t = v4l2_input_device::context_ptr_t;

    config_t                            m_config;

    v4l2_object_t::u_ptr_t              m_v4l2_object;

    frame_info_t::array_t               m_format_list;
    frame_info_t                        m_frame_info;


    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<context_t>(config);
    }

    context_t(const config_t& config)
        : m_config(config)
        , m_v4l2_object(nullptr)
    {

    }

    ~context_t()
    {
        close();
    }

    void reset_info()
    {
        m_frame_info = {};
        m_format_list.clear();
    }

    bool load_info()
    {
        if (is_opened())
        {
            reset_info();
            if (m_v4l2_object->fetch_frame_info(m_frame_info))
            {
                m_format_list = m_v4l2_object->fetch_supported_format_list();
                return true;
            }

        }

        return false;
    }

    bool check_format_support(const frame_info_t& format) const
    {
        for (const auto& f : m_format_list)
        {
            if (f == format)
            {
                return true;
            }
        }
        return false;
    }

    bool open()
    {
        if (m_v4l2_object == nullptr)
        {
            m_v4l2_object = v4l2_object_t::create(m_config.url
                                                  , m_config.buffers
                                                  , m_config.frame_info);
            if (m_v4l2_object != nullptr)
            {
                if (load_info())
                {
                    return true;
                }
            }
            reset_info();
            m_v4l2_object.reset();
        }

        return false;
    }

    bool close()
    {
        if (is_opened())
        {
            m_v4l2_object.reset();

            reset_info();

            return true;
        }

        return false;
    }

    inline bool is_opened() const
    {
        return m_v4l2_object != nullptr;
    }

    bool set_config(const config_t& config)
    {
        if (is_opened())
        {
            m_config = config;
            return true;
        }
        return false;
    }

    const config_t& config() const
    {
        return m_config;
    }

    frame_info_t::array_t get_supported_formats() const
    {
        return m_format_list;
    }

    frame_info_t get_format() const
    {
        return m_frame_info;
    }

    bool set_format(const frame_info_t& format)
    {
        if (is_opened())
        {
            if (m_frame_info == format)
            {
                return true;
            }

            if (check_format_support(format))
            {
                m_v4l2_object.reset();
                m_v4l2_object = v4l2_object_t::create(m_config.url
                                                      , m_config.buffers
                                                      , format);
                if (is_opened())
                {
                    m_frame_info = format;
                    return m_v4l2_object->fetch_frame_info(m_frame_info);
                }
            }
        }

        return false;
    }

    control_info_t::map_t get_supported_controls() const
    {
        if (is_opened())
        {
            return m_v4l2_object->fetch_control_list();
        }

        return {};
    }

    std::size_t controls(ctrl_command_t::array_t& controls)
    {
        std::size_t result = 0;

        if (is_opened())
        {
            for (auto& c : controls)
            {
                if (c.id >= ctrl_base)
                {
                    if (m_v4l2_object->control(c))
                    {
                        result++;
                    }
                }
            }
        }

        return result;
    }

    bool control(ctrl_command_t& control)
    {
        return is_opened()
                && m_v4l2_object->control(control);
    }

    bool read_frame(frame_t& frame)
    {
        if (is_opened())
        {
            frame.frame_data = m_v4l2_object->fetch_frame_data(m_config.read_timeout);
            if (!frame.frame_data.empty())
            {
                frame.frame_info = m_frame_info;
                return true;
            }
        }

        return false;
    }
};

v4l2_input_device::config_t::config_t(const std::string_view &url
                                      , std::size_t buffers
                                      , uint32_t read_timeout
                                      , const frame_info_t& frame_info)
    : url(url)
    , buffers(buffers)
    , read_timeout(read_timeout)
    , frame_info(frame_info)
{

}

v4l2_input_device::v4l2_input_device(const config_t &config)
    : m_context(context_t::create(config))
{

}

v4l2_input_device::~v4l2_input_device()
{

}

bool v4l2_input_device::open()
{
    return m_context->open();
}

bool v4l2_input_device::close()
{
    return m_context->close();
}

bool v4l2_input_device::is_opened() const
{
    return m_context->is_opened();
}

bool v4l2_input_device::set_config(const config_t &config)
{
    return m_context->set_config(config);
}

const v4l2_input_device::config_t &v4l2_input_device::config() const
{
    return m_context->config();
}

frame_info_t::array_t v4l2_input_device::get_supported_formats() const
{
    return m_context->get_supported_formats();
}

bool v4l2_input_device::set_format(const frame_info_t &format)
{
    return m_context->set_format(format);
}

frame_info_t v4l2_input_device::get_format() const
{
    return m_context->get_format();
}

control_info_t::map_t v4l2_input_device::get_supported_controls() const
{
    return m_context->get_supported_controls();
}

std::size_t v4l2_input_device::controls(ctrl_command_t::array_t &controls)
{
    return m_context->controls(controls);
}

bool v4l2_input_device::control(ctrl_command_t &control)
{
    return m_context->control(control);
}

bool v4l2_input_device::read_frame(frame_t &frame)
{
    return m_context->read_frame(frame);
}

}
