#include "v4l2_device.h"
#include "v4l2_api.h"

#include <thread>
#include <atomic>
#include <chrono>
#include <mutex>
#include <map>
#include <future>

#define WBS_MODULE_NAME "v4l2:device"
#include "tools/base/logger_base.h"


namespace v4l2
{

const std::int32_t relative_min = -0x80000000;
const std::int32_t relative_max = 0x7fffffff;

const std::uint32_t watchdog_timeout = 5000;
const std::size_t max_frame_queue = 10;

template<typename T>
static T scale_value(T input_value, T input_min, T input_max, T output_min, T output_max)
{
        auto k = (output_max - output_min) / (input_max - input_min);
        auto b = output_min - input_min * k;

        return input_value * k + b;
}


struct command_controller_t
{

    struct request_t
    {
        std::uint32_t               control_id;
        std::int32_t                value;
        bool                        is_set;
        bool                        is_relative;

        std::promise<std::int32_t>  promise;

        request_t(std::uint32_t control_id
                  , std::int32_t value
                  , bool is_relative = false)
            : control_id(control_id)
            , value(value)
            , is_set(true)
            , is_relative(is_relative)
        {

        }

        request_t(std::uint32_t control_id
                  , bool is_relative = false)
            : control_id(control_id)
            , value(0)
            , is_set(false)
            , is_relative(is_relative)
        {

        }

        void set_result(std::int32_t value = 0)
        {
            promise.set_value(value);
        }

        bool get_result(std::uint32_t& value
                        , std::uint32_t timeout)
        {
            auto future = promise.get_future();

            if (future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready)
            {
                value = future.get();
                return true;
            }

            return false;
        }
    };

    typedef std::queue<request_t> request_queue_t;

    std::atomic_bool            is_request;
    std::mutex                  mutex;
    request_queue_t             request_queue;

    command_controller_t()
        : is_request(false)
    {

    }

    bool set_control(std::uint32_t control_id
                     , std::int32_t value
                     , std::uint32_t timeout
                     , bool is_relative = false)
    {
        request_t request(control_id
                          , value
                          , is_relative);

        auto future = request.promise.get_future();

        {
            std::lock_guard<std::mutex> lg(mutex);
            request_queue.emplace(std::move(request));
            is_request = true;
        }

        return future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready;
    }

    bool get_control(std::uint32_t control_id
                     , std::int32_t& value
                     , std::uint32_t timeout
                     , bool is_relative = false)
    {
        request_t request(control_id
                          , is_relative);

        auto future = request.promise.get_future();

        {
            std::lock_guard<std::mutex> lg(mutex);
            request_queue.emplace(std::move(request));
            is_request = true;
        }

        auto result = future.wait_for(std::chrono::milliseconds(timeout)) == std::future_status::ready;

        if (result)
        {
            value = future.get();
        }

        return result;
    }

    request_queue_t fetch_request_queue()
    {
        if (is_request)
        {
            std::lock_guard<std::mutex> lg(mutex);
            is_request = false;
            return std::move(request_queue);
        }

        return request_queue_t();
    }
};

struct v4l2_object_t
{
    std::int32_t handle;
    mapped_buffer_t mapped_buffer;

    v4l2_object_t(const std::string& uri
                  , const frame_info_t& frame_info = frame_info_t()
                  , std::size_t buffer_count = 1)

        : handle(v4l2::open_device(uri))
    {
        if (handle >= 0)
        {
            if (!frame_info.is_null())
            {
                set_frame_info(frame_info);
            }

            mapped_buffer = std::move(std::move(v4l2::map(handle
                                                          , buffer_count)
                                                ));
        }
    }

    ~v4l2_object_t()
    {
        v4l2::unmap(handle
              , mapped_buffer);
        v4l2::close_device(handle);
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

    format_list_t fetch_supported_format_list()
    {
        return std::move(v4l2::fetch_supported_format(handle));
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

        return set_frame_format(frame_info.size
                                , frame_info.pixel_format)
                && set_fps(frame_info.fps);

    }

    control_map_t fetch_control_list()
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

    bool set_control(std::uint32_t control_id, std::int32_t value)
    {
        return v4l2::set_control(handle, control_id, value);
    }

    bool get_control(std::uint32_t control_id, std::int32_t& value)
    {
        return v4l2::get_control(handle, control_id, value);
    }

};


struct v4l2_device_context_t
{    
    typedef std::queue<std::pair<std::uint32_t, std::int32_t>> control_queue_t;

    frame_handler_t                     m_frame_handler;
    stream_event_handler_t              m_stream_event_handler;

    std::thread                         m_stream_thread;
    mutable std::mutex                  m_mutex;

    format_list_t                       m_format_list;
    control_map_t                       m_control_list;
    frame_info_t                        m_frame_info;
    frame_queue_t                       m_frame_queue;

    control_queue_t                     m_control_queue;
    command_controller_t                m_command_controller;

    std::atomic_bool                    m_running;
    std::size_t                         m_frame_counter;
    std::unique_ptr<v4l2_object_t>      m_device;

    bool                                m_control_support;

    v4l2_device_context_t(frame_handler_t frame_handler
                          , stream_event_handler_t stream_event_handler)
        : m_frame_handler(frame_handler)
        , m_stream_event_handler(stream_event_handler)
        , m_running(false)
        , m_frame_counter(0)
        , m_control_support(false)
    {

    }

    ~v4l2_device_context_t()
    {
        close();
    }

    bool open(const std::string &uri
              , std::uint32_t buffer_count)
    {
        close();

        m_running = true;
        m_stream_thread = std::thread(&v4l2_device_context_t::stream_proc
                                      , this
                                      , uri
                                      , buffer_count);

        return m_running;
    }

    bool close()
    {
        if (m_running)
        {
            m_running = false;

            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
            }

        }
        return false;
    }

    bool is_opened() const
    {
        return m_running;
    }

    bool is_established() const
    {
        return m_frame_counter > 0;
    }

    format_list_t get_supported_formats() const
    {
        std::lock_guard<std::mutex> lg(m_mutex);        
        return m_format_list;
    }

    control_list_t get_control_list() const
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        control_list_t control_list;

        for (const auto& c: m_control_list)
        {
            control_list.push_back(c.second);
        }

        return std::move(control_list);
    }

    bool open_device(std::string uri
                     , std::uint32_t buffer_count
                     , frame_info_t& frame_info)
    {
        m_control_support = false;

        if (uri.find("v4l2://") == 0)
        {
            uri = uri.substr(6);
        }

        std::lock_guard<std::mutex> lg(m_mutex);
        m_device.reset();                
        m_device.reset(new v4l2_object_t(uri
                                     , frame_info
                                     , buffer_count));


        if (m_device->fetch_frame_info(frame_info))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            auto formats = m_device->fetch_supported_format_list();
            auto controls = m_device->fetch_control_list();

            // formats.push_back(frame_info_t({ 800, 600 }, 15, pixel_format_h264));
            m_format_list = std::move(formats);
            m_control_list = std::move(controls);

            return true;
        }

        return false;
    }


    void stream_proc(std::string uri
                     , std::uint32_t buffer_count)
    {
        push_event(streaming_event_t::start);

        while (m_running)
        {
            std::uint32_t frame_time = 50;


            if (open_device(uri
                             , buffer_count
                             , m_frame_info))
            {
                frame_info_t frame_info = m_frame_info;
                frame_time = frame_info.fps == 0 ? 100 : (1000 / frame_info.fps);

                push_event(streaming_event_t::open);

                auto tp = std::chrono::high_resolution_clock::now();

                while(m_running
                      && m_frame_info == frame_info)
                {        
                    command_process(*m_device);

                    frame_t frame(frame_info
                                  , std::move(m_device->fetch_frame_data(frame_time * 2)));

                    if (!frame.frame_data.empty())
                    {
                        m_frame_counter++;
                        tp = std::chrono::high_resolution_clock::now();
                        if (m_frame_handler == nullptr
                                || m_frame_handler(std::move(frame)) == false)
                        {                       
                            push_media_queue(std::move(frame));
                        }
                    }
                    else
                    {
                         auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - tp).count();
                         if (dt < watchdog_timeout)
                         {
                             std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
                         }
                         else
                         {
                            break;
                         }
                    }

                }

                m_frame_counter = 0;
                push_event(streaming_event_t::close);
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
            }
        }

        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_device.reset();
        }
        push_event(streaming_event_t::stop);
    }

    void command_process(v4l2_object_t& v4l2_object)
    {
        auto requests = m_command_controller.fetch_request_queue();

        while(!requests.empty())
        {
            auto& request = requests.front();

            std::int32_t min = 0;
            std::int32_t max = 0;

            if (request.is_relative)
            {
                auto it = m_control_list.find(request.control_id);

                if (it == m_control_list.end())
                {
                    break;
                }

                min = it->second.range.min;
                max = it->second.range.max;
            }

            if (request.is_set)
            {
                if (request.is_relative)
                {
                    request.value = scale_value<double>(request.value, relative_min, relative_max, min, max);
                }

                v4l2_object.set_control(request.control_id
                                        , request.value);
                request.set_result();
            }
            else
            {
                v4l2_object.get_control(request.control_id
                                        , request.value);

                if (request.is_relative)
                {
                    request.value = scale_value<double>(request.value, min, max, relative_min, relative_max);
                }

                request.set_result(request.value);
            }

            requests.pop();
        }
    }

    void push_event(streaming_event_t capture_event)
    {
        if (m_stream_event_handler != nullptr)
        {
            m_stream_event_handler(capture_event);
        }
    }

    frame_queue_t fetch_media_queue()
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return std::move(m_frame_queue);
    }

    void push_media_queue(frame_t&& frame)
    {
        if (!frame.frame_data.empty())
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_frame_queue.emplace(std::move(frame));

            while (m_frame_queue.size() > max_frame_queue)
            {
                m_frame_queue.pop();
            }
        }
    }


    bool set_control(std::uint32_t control_id, std::int32_t value)
    {

        return m_command_controller.set_control(control_id
                                                , value
                                                , 100
                                                , false);

    }

    bool set_relative_control(std::uint32_t control_id, double value)
    {

        return m_command_controller.set_control(control_id
                                                , scale_value<double>(value, 0.0, 1.0, relative_min, relative_max)
                                                , 100
                                                , true);
    }


    std::int32_t get_control(std::uint32_t control_id, std::int32_t default_value = 0)
    {
        m_command_controller.get_control(control_id
                                         , default_value
                                         , 100
                                         , false);

        return default_value;
    }

    double get_relative_control(std::uint32_t control_id, double default_value = 0)
    {
        std::int32_t real_value = 0;

        if (m_command_controller.get_control(control_id
                                         , real_value
                                         , 100
                                         , true))
        {
            default_value = scale_value<double>(real_value, relative_min, relative_max, 0.0, 1.0);
        }

        return default_value;

    }

    bool set_ptz(double pan
                 , double tilt
                 , double zoom)
    {
        if (is_opened())
        {
            bool result = set_relative_control(ctrl_pan_absolute, pan);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            result &= set_relative_control(ctrl_tilt_absolute, tilt);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            result &= set_relative_control(ctrl_zoom_absolute, zoom);
        }
        return true;
    }

    bool get_ptz(double& pan
                 , double& tilt
                 , double& zoom)
    {
        if (is_opened())
        {
            pan = get_relative_control(ctrl_pan_absolute);
            tilt = get_relative_control(ctrl_tilt_absolute);
            zoom = get_relative_control(ctrl_zoom_absolute);
        }
        return true;
    }

};
//------------------------------------------------------------------------------------------
void v4l2_device_context_deleter_t::operator()(v4l2_device_context_t *v4l2_device_context_ptr)
{
    delete v4l2_device_context_ptr;
}
//---------------------------------------------------------------------------------------------
v4l2_device::v4l2_device(frame_handler_t frame_handler
        , stream_event_handler_t stream_event_handler)
    : m_v4l2_device_context(new v4l2_device_context_t(frame_handler
                                                      , stream_event_handler))
{

}

bool v4l2_device::open(const std::string &uri
                       , std::uint32_t buffer_count)
{
    return m_v4l2_device_context->open(uri
                                        , buffer_count);
}

bool v4l2_device::close()
{
    return m_v4l2_device_context->close();
}

bool v4l2_device::is_opened() const
{
    return m_v4l2_device_context->is_opened();
}

bool v4l2_device::is_established() const
{
    return m_v4l2_device_context->is_established();
}

format_list_t v4l2_device::get_supported_formats() const
{
    return std::move(m_v4l2_device_context->get_supported_formats());
}

const frame_info_t &v4l2_device::get_format() const
{
    return m_v4l2_device_context->m_frame_info;
}

bool v4l2_device::set_format(const frame_info_t &format)
{
    m_v4l2_device_context->m_frame_info = format;
    return true;
}

control_list_t v4l2_device::get_control_list() const
{
    return std::move(m_v4l2_device_context->get_control_list());
}

bool v4l2_device::set_control(uint32_t control_id
                              , int32_t value)
{
    return m_v4l2_device_context->set_control(control_id
                                              , value);
}

int32_t v4l2_device::get_control(uint32_t control_id
                                 , int32_t default_value)
{
    return m_v4l2_device_context->get_control(control_id
                                              , default_value);
}

bool v4l2_device::set_relative_control(uint32_t control_id, double value)
{
    return m_v4l2_device_context->set_relative_control(control_id
                                                      , value);
}

double v4l2_device::get_relatuive_control(uint32_t control_id, double default_value)
{
    return m_v4l2_device_context->get_relative_control(control_id
                                                       , default_value);
}

bool v4l2_device::get_ptz(double &pan
                          , double &tilt
                          , double &zoom)
{
    return m_v4l2_device_context->get_ptz(pan
                                          , tilt
                                          , zoom);
}

bool v4l2_device::set_ptz(double pan
                          , double tilt
                          , double zoom)
{
    return m_v4l2_device_context->set_ptz(pan
                                          , tilt
                                          , zoom);
}

frame_queue_t v4l2_device::fetch_media_queue()
{
    return std::move(m_v4l2_device_context->fetch_media_queue());
}

}
