#include "libav_output_device_factory.h"

#include "utils/message_router_impl.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "core/event_channel_state.h"
#include "utils/time_utils.h"
#include "utils/common_utils.h"

#include "utils/option_helper.h"

#include "media/media_message_types.h"
#include "media/media_option_types.h"
#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"

#include "tools/utils/sync_base.h"
//#include "tools/ffmpeg/libav_stream_publisher.h"
#include "tools/ffmpeg/libav_output_format.h"

#include <shared_mutex>
#include <thread>
#include <atomic>
#include <queue>

#include <iostream>

namespace mpl::media
{

constexpr std::size_t default_max_queue_size = 1000;
constexpr std::size_t default_max_repeat_errors = 10;

using stream_list_t = std::vector<i_media_format::u_ptr_t>;
using stream_map_t = std::map<std::int32_t, i_media_format::u_ptr_t>;

namespace detail
{
    template<typename Format>
    bool check_and_append_format(Format&& format
                                , stream_list_t& streams)
    {
        if (format != nullptr
                && format->is_valid())
        {
            streams.emplace_back(std::move(format));
            return true;
        }

        return false;
    }

    bool deserialize_stream_list(const i_property& property
                                 , stream_list_t& streams)
    {
        bool result = false;
        if (property.property_type() == property_type_t::array)
        {
            for (const auto& f : static_cast<const i_property_array&>(property).get_value())
            {
                if (f != nullptr)
                {
                    switch(property_reader(*f).get("media_type", media_type_t::undefined))
                    {
                        case media_type_t::audio:
                        {
                            result |= check_and_append_format(audio_format_impl::create(*f)
                                                              , streams);
                        }
                        break;
                        case media_type_t::video:
                        {
                            result |= check_and_append_format(video_format_impl::create(*f)
                                                              , streams);
                        }
                        break;
                        default:;
                    }
                }
            }

        }

        return result;
    }

    i_property::u_ptr_t serialize_stream_list(const stream_list_t& streams_list)
    {
        i_property::s_array_t streams;

        for (const auto& s : streams_list)
        {
            if (s != nullptr)
            {
                switch(s->media_type())
                {
                    case media_type_t::audio:
                    {
                        const audio_format_impl& audio_format = static_cast<const audio_format_impl&>(*s);
                        if (auto p = property_helper::create_object())
                        {
                            if (audio_format.get_params(*p))
                            {
                                streams.emplace_back(std::move(p));
                            }
                        }
                    }
                    break;
                    case media_type_t::video:
                    {
                        const video_format_impl& video_format = static_cast<const video_format_impl&>(*s);
                        if (auto p = property_helper::create_object())
                        {
                            if (video_format.get_params(*p))
                            {
                                streams.emplace_back(std::move(p));
                            }
                        }
                    }
                    break;
                    default:;
                }
            }
        }

        if (!streams.empty())
        {
            return property_helper::create_array(std::move(streams));
        }

        return nullptr;
    }

}


struct timestamp_manager_t
{
    struct stream_context_t
    {
        using list_t = std::vector<stream_context_t>;
        pt::ffmpeg::stream_info_t       stream;
        std::size_t                     frame_id;
        timestamp_t                     first_timestamp;

        stream_context_t(const pt::ffmpeg::stream_info_t& stream_info)
            : stream(stream_info)
            , frame_id(0)
            , first_timestamp(0)
        {

        }

        void reset()
        {
            frame_id = 0;
            first_timestamp = 0;
        }

        timestamp_t push_pts(timestamp_t native_timestamp)
        {
            if (frame_id == 0)
            {
                first_timestamp = native_timestamp;
            }

            frame_id++;
            return native_timestamp - first_timestamp;
        }
    };

    stream_context_t::list_t        streams;
    bool                            open;

    static stream_context_t::list_t create_stream_context_list(const pt::ffmpeg::stream_info_list_t& streams)
    {
        stream_context_t::list_t stream_context_list;

        for (const auto& s : streams)
        {
            stream_context_list.emplace_back(stream_context_t(s));
        }

        return stream_context_list;
    }

    timestamp_manager_t(const pt::ffmpeg::stream_info_list_t& streams)
        : streams(create_stream_context_list(streams))
        , open(false)
    {

    }

    void reset()
    {
        for (auto& s : streams)
        {
            s.reset();
        }
    }

    bool has_sync() const
    {
        bool result = !streams.empty();

        for (const auto& s : streams)
        {
            result &= s.frame_id > 0;
        }

        return result;
    }

    bool check_and_rescale_frame(pt::ffmpeg::frame_t& libav_frame)
    {
        if (static_cast<std::size_t>(libav_frame.info.stream_id) < streams.size())
        {
            auto& stream = streams[libav_frame.info.stream_id];
            libav_frame.info.pts = stream.push_pts(libav_frame.info.timestamp());
            libav_frame.info.dts = libav_frame.info.pts;

            if (!open)
            {
                if (has_sync())
                {
                    reset();
                    open = true;
                }
            }

            return open;
        }

        return false;
    }
};

class libav_output_device : public i_device
{
    using thread_t = std::thread;
    using mutex_t = pt::utils::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;
    using libav_frame_queue_t = std::queue<pt::ffmpeg::frame_t>;

    struct device_params_t
    {
        device_type_t   device_type = device_type_t::libav_out;
        std::string     url;
        std::string     options;
        stream_list_t   streams;
        bool            sync_tracks;

        device_params_t(device_type_t device_type = device_type_t::libav_out
                , const std::string_view& url = {}
                , const std::string_view& options = {}
                , stream_list_t&& streams = {})
            : device_type(device_type)
            , url(url)
            , options(options)
            , streams(std::move(streams))
            , sync_tracks(false)
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
            if (reader.get("url", url))
            {
                reader.get("options", options);
                if (auto s  = reader["streams"])
                {
                    streams.clear();
                    return detail::deserialize_stream_list(*s
                                                           , streams);
                }
            }

            return false;
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            if (writer.set("url", url))
            {
                writer.set("options", options, {});
                if (auto s = detail::serialize_stream_list(streams))
                {
                    return writer.set("streams", *s);
                }
            }

            return false;
        }

        pt::ffmpeg::stream_info_list_t native_streams() const
        {
            pt::ffmpeg::stream_info_list_t stream_list;
            track_id_t stream_id = 0;
            for (const auto& s : streams)
            {
                pt::ffmpeg::stream_info_t stream_info;
                if (s != nullptr
                        && utils::convert(*s
                                          , stream_info))
                {
                    stream_info.stream_id = stream_id;
                    stream_list.emplace_back(std::move(stream_info));
                }
                stream_id++;
            }

            return stream_list;
        }

        const i_media_format* get_format(track_id_t track_id) const
        {
            if (static_cast<std::size_t>(track_id) < streams.size())
            {
                return streams[track_id].get();
            }
            return nullptr;
        }

        inline track_id_t get_track_id(const i_media_format& format) const
        {
            track_id_t track_id = 0;
            for (const auto& s : streams)
            {
                if (s->is_compatible(format))
                {
                    return track_id;
                }
                track_id++;
            }

            return track_id_undefined;
            /*
            auto found_track_id = option_reader(format.options()).get(opt_fmt_track_id
                                                                      , track_id_undefined);
            if (found_track_id == track_id_undefined)
            {
                track_id_t track_id = 0;
                for (const auto& s : streams)
                {
                    if (s->is_compatible(format))
                    {
                        return track_id;
                    }
                    track_id++;
                }
            }
            else if (auto f = get_format(found_track_id))
            {
                if (f->is_compatible(format))
                {
                    return found_track_id;
                }
            }

            return track_id_undefined;*/
        }

        pt::ffmpeg::libav_output_format::config_t native_config() const
        {
            return
            {
                url,
                options,
                native_streams()
            };
        }

        bool is_valid() const
        {
            return device_type == device_type_t::libav_out
                    && !url.empty()
                    && !streams.empty();
        }
    };

    class frame_manager
    {
        mutable mutex_t             m_safe_mutex;

        libav_frame_queue_t         m_frame_queue;
        std::size_t                 m_max_queue_size;
    public:
        frame_manager(std::size_t max_queue_size = default_max_queue_size)
            : m_max_queue_size(max_queue_size)
        {

        }

        ~frame_manager()
        {
            clear();
        }

        void push_frame(pt::ffmpeg::frame_t&& libav_frame)
        {
            lock_t lock(m_safe_mutex);
            m_frame_queue.emplace(std::move(libav_frame));
            while(m_frame_queue.size() > m_max_queue_size)
            {
                m_frame_queue.pop();
            }
        }

        bool fetch_frame(pt::ffmpeg::frame_t& frame)
        {
            lock_t lock(m_safe_mutex);
            if (!m_frame_queue.empty())
            {
                frame = std::move(m_frame_queue.front());
                m_frame_queue.pop();
                return true;
            }

            return false;
        }

        libav_frame_queue_t fetch_frames()
        {
            lock_t lock(m_safe_mutex);
            return std::move(m_frame_queue);
        }

        void clear()
        {
            lock_t lock(m_safe_mutex);
            m_frame_queue = {};
        }
    };


    mutable mutex_t                 m_safe_mutex;
    thread_t                        m_thread;

    device_params_t                 m_device_params;
    message_sink_impl               m_message_sink;
    message_router_impl             m_router;

    frame_manager                   m_frame_manager;

    channel_state_t                 m_state;
    std::atomic_bool                m_open;

public:
    using u_ptr_t = std::unique_ptr<libav_output_device>;
    using s_ptr_t = std::shared_ptr<libav_output_device>;

    static u_ptr_t create(const i_property& params)
    {
        device_params_t device_params(params);
        if (device_params.is_valid())
        {
            return std::make_unique<libav_output_device>(std::move(device_params));
        }

        return nullptr;
    }

    libav_output_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_message_sink([&](const auto& message) { return on_message(message); } )
        , m_state(channel_state_t::ready)
        , m_open(false)

    {

    }

    ~libav_output_device()
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
                                           , true
                                           , std::memory_order_acquire))
        {
            change_state(channel_state_t::opening);

            m_thread = std::thread([&]{ main_proc(); });

            return true;
        }

        return false;
    }

    bool close()
    {
        if (m_open.load(std::memory_order_acquire))
        {
            change_state(channel_state_t::closing);

            m_open.store(false
                         , std::memory_order_release);

            if (m_thread.joinable())
            {
                m_thread.join();
            }

            change_state(channel_state_t::closed);

            return true;
        }

        return false;
    }

    bool on_audio_frame(const i_audio_frame& frame)
    {
        auto stream_id = m_device_params.get_track_id(frame.format());
        if (stream_id != track_id_undefined)
        {
            if (auto buffer = frame.data().get_buffer(media_buffer_index))
            {
                pt::ffmpeg::frame_t libav_frame;
                libav_frame.info.media_info.media_type = pt::ffmpeg::media_type_t::audio;
                libav_frame.info.stream_id = stream_id;
                libav_frame.media_data = utils::create_raw_array(buffer->data()
                                                                 , buffer->size());
                libav_frame.info.pts = frame.timestamp();
                libav_frame.info.dts = libav_frame.info.pts;

                std::clog << "audio #" << frame.frame_id()
                          << ", ts: " << frame.timestamp()
                          << ", sz: " << buffer->size()
                          << std::endl;

                m_frame_manager.push_frame(std::move(libav_frame));

                return true;
            }
        }

        return false;
    }

    bool on_video_frame(const i_video_frame& frame)
    {
        auto stream_id = m_device_params.get_track_id(frame.format());
        if (stream_id != track_id_undefined)
        {
            if (auto buffer = frame.data().get_buffer(media_buffer_index))
            {
                pt::ffmpeg::frame_t libav_frame;
                libav_frame.info.media_info.media_type = pt::ffmpeg::media_type_t::video;
                libav_frame.info.stream_id = stream_id;
                libav_frame.media_data = utils::create_raw_array(buffer->data()
                                                                 , buffer->size());
                libav_frame.info.pts = frame.timestamp();
                libav_frame.info.dts = libav_frame.info.pts;
                libav_frame.info.key_frame = frame.frame_type() == i_video_frame::frame_type_t::key_frame;

                std::clog << "video #" << frame.frame_id() << ", ts: " << frame.timestamp() << std::endl;

                m_frame_manager.push_frame(std::move(libav_frame));

                return true;
            }
        }

        return false;
    }

    bool on_message_frame(const i_media_frame& media_frame)
    {
        if (m_open)
        {
            switch(media_frame.media_type())
            {
                case media_type_t::audio:
                    return on_audio_frame(static_cast<const i_audio_frame&>(media_frame));
                break;
                case media_type_t::video:
                    return on_video_frame(static_cast<const i_video_frame&>(media_frame));
                break;
                default:;
            }

        }
        return false;
    }

    bool on_message(const i_message& message)
    {
        switch(message.category())
        {
            case message_category_t::data:
            if (static_cast<const i_message_data&>(message).subclass() == message_class_media
                    && static_cast<const i_message_media_data&>(message).data_type() == media_data_type_t::frame)
            {
                return on_message_frame(static_cast<const i_media_frame&>(message));
            }
            break;
            default:;
        }

        return false;
    }

    void main_proc()
    {
        change_state(channel_state_t::open);

        while(libav_output_device::is_open())
        {
            pt::ffmpeg::libav_output_format::config_t native_config = m_device_params.native_config();

            pt::ffmpeg::libav_output_format native_publisher(native_config);
            timestamp_manager_t pts_manager(native_config.streams);

            change_state(channel_state_t::connecting);
            if (native_publisher.open())
            {
                std::size_t err_count = 0;
                change_state(channel_state_t::connected);

                while(libav_output_device::is_open())
                {
                    pt::ffmpeg::frame_t libav_frame;
                    while(m_frame_manager.fetch_frame(libav_frame)
                          && libav_output_device::is_open())
                    {
                        if (pts_manager.check_and_rescale_frame(libav_frame))
                        {
                            if (!native_publisher.write(libav_frame.get_frame_ref()))
                            {
                                err_count++;
                                break;
                            }
                            else
                            {
                                err_count = 0;
                            }
                        }
                    }
                    if (err_count > default_max_repeat_errors)
                    {
                        break;
                    }

                    std::this_thread::sleep_for(std::chrono::milliseconds(50));
                }
                change_state(channel_state_t::disconnecting);
            }
            change_state(channel_state_t::disconnected);
            native_publisher.close();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        change_state(channel_state_t::closed);
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

    // i_message_channel interface
public:
    i_message_sink *sink(std::size_t index) override
    {
        if (index == 0)
        {
            return &m_message_sink;
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
        return device_type_t::libav_out;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        if (!m_open)
        {
            device_params_t device_params;
            if (device_params.load(params)
                    && device_params.is_valid())
            {
                m_device_params = std::move(device_params);
                return true;
            }
        }

        return false;
    }

    bool get_params(i_property &params) const override
    {
        return m_device_params.save(params);
    }
};

libav_output_device_factory::libav_output_device_factory()
{

}

i_device::u_ptr_t libav_output_device_factory::create_device(const i_property &device_params)
{
    return libav_output_device::create(device_params);
}

}