#include "libav_stream_grabber.h"
#include "libav_utils.h"

#include <thread>
#include <mutex>
#include <map>
#include <atomic>
#include <chrono>

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}

#define WBS_MODULE_NAME "ff:grabber"
#include "tools/base/logger_base.h"

#include <iostream>
#include "tools/base/string_base.h"
#include "tools/base/url_base.h"

namespace ffmpeg
{

const std::size_t max_queue_size = 1000;
const std::size_t min_queue_size = max_queue_size / 10;
const std::size_t idle_timeout_ms = 10;
const std::size_t reconnect_timeout_ms = 500;
const std::uint64_t read_timeout_ms = 5000;
const std::size_t max_repetitive_errors = 10;

//------------------------------------------------------------------------------------

struct libav_input_format_context_t
{

struct interrupt_timeout_t
{
    std::uint64_t end_tp = 0;
    std::atomic_bool&   running_flag;

    interrupt_timeout_t(std::atomic_bool& running_flag)
        : running_flag(running_flag)
    {

    }

    void reset(std::uint64_t timeout = read_timeout_ms)
    {
        end_tp = adaptive_timer_t::now() + timeout;
    }

    bool is_timeout() const
    {
        return end_tp < adaptive_timer_t::now();
    }

    bool is_interrupt() const
    {
        return !running_flag.load(std::memory_order_consume) || is_timeout();
    }

    static std::int32_t check_interrupt(void* timeout_context)
    {
        return timeout_context != nullptr
                && static_cast<interrupt_timeout_t*>(timeout_context)->is_interrupt();
    }
};

struct AVFormatContext*     context;
struct AVPacket             packet;
std::uint32_t               context_id;
std::size_t                 total_read_bytes;
std::size_t                 total_read_frames;
device_type_t               type;
bool                        is_file;
bool                        is_streaming_protocol;
bool                        is_sync_tracks;
interrupt_timeout_t         interrupt_timeout;
std::int64_t                timestamp_offset;

bool                        is_init;

libav_input_format_context_t(std::atomic_bool& running_flag)
    : context(nullptr)
    , context_id(0)
    , total_read_bytes(0)
    , total_read_frames(0)
    , type(device_type_t::unknown)
    , is_file(false)
    , is_streaming_protocol(false)
    , is_sync_tracks(false)
    , interrupt_timeout(running_flag)
    , timestamp_offset(0)
    , is_init(false)
{
    static std::uint32_t ctx_id = 0;
    context_id = ctx_id++;

    LOG_T << "Context #" << context_id << ". Create" LOG_END;

    av_init_packet(&packet);
}
~libav_input_format_context_t()
{
    LOG_T << "Context #" << context_id << ". Destroy" LOG_END;

    av_packet_unref(&packet);

    if (context != nullptr)
    {
        // av_read_pause(context);
        avformat_close_input(&context);
        avformat_free_context(context);

        LOG_I << "Context #" << context_id << ". Free resources" LOG_END;
    }
}

std::int32_t init(const std::string& uri
                  , const std::string& options)
{
    std::int32_t result = -1;

    auto c_uri = uri.c_str();

    auto device_type = utils::fetch_device_type(uri);

    base::url_info_t url_info;

    url_info.parse_url(uri);

    is_streaming_protocol = device_type == device_type_t::rtmp
            || device_type == device_type_t::rtsp
            || device_type == device_type_t::rtp
            || device_type == device_type_t::alsa
            || device_type == device_type_t::pulse;


    if (!is_init)
    {
        AVDictionary* av_options = nullptr;

        AVInputFormat *input_format = nullptr;
        if (url_info.is_valid())
        {
            input_format = av_find_input_format(url_info.protocol.c_str());
        }

        type = device_type;
        switch(type)
        {
            case device_type_t::rtsp:
                av_dict_set_int(&av_options, "stimeout", 1000000, 0);
            break;
            case device_type_t::camera:
                av_dict_set(&av_options, "pixel_format", "mjpeg", 0);
                c_uri += 6;
                if (*c_uri != '/')
                {
                    c_uri++;
                }
            break;
            case device_type_t::alsa:
            case device_type_t::pulse:
                if (url_info.is_valid())
                {
                    c_uri = url_info.host.c_str();
                }
            break;
            default:
                av_dict_set_int(&av_options, "rw_timeout", read_timeout_ms * 1000, 0);
            break;
        }


        utils::set_options(av_options
                           , options);

        context = avformat_alloc_context();

        interrupt_timeout.reset();
        context->interrupt_callback.opaque = &interrupt_timeout;
        context->interrupt_callback.callback = &interrupt_timeout_t::check_interrupt;
        context->flags |= AVFMT_FLAG_NONBLOCK;

        result = avformat_open_input(&context
                                     , c_uri
                                     , input_format
                                     , &av_options);

        av_dict_free(&av_options);

        if (result == 0)
        {
            result = avformat_find_stream_info(context
                                               , nullptr);

            is_file = (context->iformat->flags & AVFMT_NOFILE) == 0;

            if (device_type == device_type_t::rtmp) // temp hack
            {
                is_file = false;
            }

            is_init = result >= 0;

            if (is_init)
            {
                if (context->nb_streams > 1)
                {
                    AVRational time_base = context->streams[0]->time_base;
                    for (auto i = 1; i < context->nb_streams; i++)
                    {
                        if (time_base.den == context->streams[i]->time_base.den
                                && time_base.num == context->streams[i]->time_base.num)
                        {
                            is_sync_tracks = true;
                            continue;
                        }
                        is_sync_tracks = false;
                        break;
                    }
                }
                LOG_I << "Context #" << context_id << ". Open streams (" << context->nb_streams << ") success" LOG_END;
            }
            else
            {
                LOG_E << "Context #" << context_id << ". Open streams failed, err = " << result LOG_END;
            }
        }
    }

    return result;
}

stream_info_list_t get_streams(stream_mask_t stream_mask)
{
    stream_info_list_t streams;

    bool is_audio_allowed = (stream_mask & stream_mask_t::stream_mask_audio) != stream_mask_empty;
    bool is_video_allowed = (stream_mask & stream_mask_t::stream_mask_video) != stream_mask_empty;
    bool is_data_allowed = (stream_mask & stream_mask_t::stream_mask_data) != stream_mask_empty;    

    if (is_init)
    {
        for (unsigned i = 0; i < context->nb_streams; i++)
        {
            auto av_stream = context->streams[i];            

            stream_info_t stream_info = {};

            switch(av_stream->codec->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:

                    if (!is_audio_allowed)
                    {
                        continue;
                    }

                    stream_info.media_info.media_type = media_type_t::audio;                   

                    stream_info.media_info.audio_info.sample_rate = av_stream->codecpar->sample_rate;
                    stream_info.media_info.audio_info.channels = av_stream->codecpar->channels;
                    stream_info.media_info.audio_info.sample_format = static_cast<sample_format_t>(av_stream->codecpar->format);

                break;
                case AVMEDIA_TYPE_VIDEO:

                    if (!is_video_allowed)
                    {
                        continue;
                    }

                    stream_info.media_info.media_type = media_type_t::video;

                    stream_info.media_info.video_info.size.width = av_stream->codec->width;
                    stream_info.media_info.video_info.size.height = av_stream->codec->height;


                    stream_info.media_info.video_info.fps = av_q2d(av_stream->avg_frame_rate) + 0.5;

                    if (stream_info.media_info.video_info.fps == 0)
                    {
                        stream_info.media_info.video_info.fps = av_q2d(av_stream->r_frame_rate) + 0.5;
                    }

                    if (stream_info.media_info.video_info.fps == 0)
                    {
                        stream_info.media_info.video_info.fps = 1;
                    }

                    stream_info.media_info.video_info.pixel_format = static_cast<pixel_format_t>(av_stream->codecpar->format);
                    stream_info.codec_info.codec_params.gop = av_stream->codec->gop_size;
                break;

                case AVMEDIA_TYPE_DATA:
                case AVMEDIA_TYPE_SUBTITLE:

                    if (!is_data_allowed)
                    {
                        continue;
                    }
                    stream_info.media_info.media_type = media_type_t::data;
                break;
                default:
                    continue;
            }
            stream_info.stream_id = av_stream->index;
            stream_info.codec_info.codec_params.parse_type = av_stream->need_parsing;

            if (av_stream->codec != nullptr)
            {
                stream_info.codec_info.id = av_stream->codec->codec_id;
                stream_info.codec_info.name = stream_info.codec_info.codec_name(stream_info.codec_info.id);
                if (stream_info.codec_info.id == codec_id_first_audio)
                {
                    stream_info.codec_info.id = codec_id_none;
                    stream_info.codec_info.name.clear();
                }
                stream_info.codec_info.codec_params.bitrate = av_stream->codec->bit_rate;
                stream_info.codec_info.codec_params.frame_size = av_stream->codec->frame_size;
                stream_info.codec_info.codec_params.flags1 = av_stream->codec->flags;
                stream_info.codec_info.codec_params.flags2 = av_stream->codec->flags2;
            }

            if (av_stream->codec->extradata != nullptr
                    && av_stream->codec->extradata_size > 0)
            {

                stream_info.extra_data = std::move(stream_info_t::create_extra_data(av_stream->codec->extradata
                                                                                    , av_stream->codec->extradata_size
                                                                                    , true));

                LOG_D << "Context #" << context_id << ". Stream #" << stream_info.stream_id << " extra header: " << base::hex_dump(av_stream->codec->extradata
                                                                                                        , av_stream->codec->extradata_size) LOG_END;

            }

            streams.emplace_back(std::move(stream_info));
        }
    }

    return streams; 
}

std::int32_t fetch_media_data(frame_t& frame)
{
    std::int64_t pts = AV_NOPTS_VALUE;
    return fetch_media_data(frame, pts);
}

void seek(std::int64_t timestamp)
{
    if (is_init)
    {
        for (auto i = 0; i < context->nb_streams; i++)
        {
            av_seek_frame(context
                          , i
                          , timestamp
                          , 0);
        }
    }
}

std::int32_t fetch_media_data(frame_t& frame, std::int64_t& frame_order)
{
    std::int32_t result = -1;

    if (is_init)
    {
        interrupt_timeout.reset();
        result = av_read_frame(context, &packet);

        if (result >= 0 && packet.size > 0)
        {
            auto& stream = context->streams[packet.stream_index];

            /*if (packet.dts == 0 && packet.pts == 0)
            {
                packet.pts = av_gettime_relative();
                packet.dts = packet.pts;

                auto delay = std::min<std::uint32_t>(AV_TIME_BASE, AV_TIME_BASE * av_q2d(stream->time_base));

                std::this_thread::sleep_for(std::chrono::microseconds(delay));
            }
            else
            {
                offset_pts(timestamp_offset);
            }*/

            offset_pts(timestamp_offset);

            if (is_sync_tracks)
            {
                auto pts = timestamp();
                auto dt = pts - frame_order;
                if (std::abs(dt) > stream->time_base.den * 3)
                {

                    timestamp_offset -= dt;
                    offset_pts(-dt);
                    dt = 0;
                }
                frame_order += dt;
            }
            else
            {
                frame_order++;
            }

            switch(stream->codecpar->codec_type)
            {
                case AVMEDIA_TYPE_AUDIO:
                    av_packet_rescale_ts(&packet, stream->time_base, { 1, stream->codecpar->sample_rate });
                break;
                case AVMEDIA_TYPE_VIDEO:
                    av_packet_rescale_ts(&packet, stream->time_base, { 1, 90000 });
                break;
            }

            frame.info.dts = packet.dts;
            frame.info.pts = packet.pts;
            frame.info.stream_id = packet.stream_index;
            frame.info.key_frame = (packet.flags & AV_PKT_FLAG_KEY) != 0;

            frame.media_data.resize(packet.size);

            memcpy(frame.media_data.data()
                        , packet.data
                        , packet.size);

            total_read_bytes += packet.size;
            total_read_frames++;

            result = packet.stream_index;

        }

        LOG_D << "Context #" << context_id << ". Fetch media data size " << packet.size
              << " from stream #" << result LOG_END;

        av_packet_unref(&packet);
    }
    else
    {
        LOG_W << "Context #" << context_id << ". Cant't fetch media data, context not init" LOG_END;
    }

    return result;
}

void offset_pts(std::int64_t offset)
{
    if (packet.pts != AV_NOPTS_VALUE)
    {
        packet.pts += offset;
    }

    if (packet.dts != AV_NOPTS_VALUE)
    {
        packet.dts += offset;
    }
}

std::int64_t timestamp() const
{
    return packet.dts == AV_NOPTS_VALUE
            ? packet.pts
            : packet.dts;
}

};

//------------------------------------------------------------------------------------
class stream_delay_controller
{
    const stream_info_t&    m_stream_info;
    std::int64_t            m_last_timestamp;
    adaptive_timer_t        m_delay;

public:
    stream_delay_controller(const stream_info_t& stream_info)
        : m_stream_info(stream_info)
        , m_last_timestamp(0)
        , m_delay(1000000)
    {

    }

    void wait_pts(std::int64_t timestamp)
    {
        constexpr std::int64_t sync_window = 1000000;
        auto sample_rate = m_stream_info.media_info.sample_rate();
        if (sample_rate > 0)
        {
            auto dt = timestamp - m_last_timestamp;
            dt = (dt * 1000000) / sample_rate;

            if (dt < 0 || dt > sync_window
                    || m_delay.elapsed() > sync_window)
            {
                // std::cout << "wait_pts: dt: " << dt << ", reset delay" << std::endl;
                m_delay.reset();
            }
            else
            {
                m_delay.wait(dt);
            }
        }

        m_last_timestamp = timestamp;
    }

    void reset()
    {
        m_delay.reset();
        m_last_timestamp = 0;
    }
};

//------------------------------------------------------------------------------------
struct libav_stream_grabber_context_t
{
    struct libav_stream_t
    {

        using pointer_t = std::shared_ptr<libav_stream_t>;
        std::mutex                  queue_mutex;

        stream_info_t               stream_info;
        stream_delay_controller     delay_controller;
        frame_queue_t               frame_queue;
        std::int32_t                frame_id;
        std::int64_t                start;
        bool                        is_streaming_protocol;

        static pointer_t create(stream_info_t&& stream_info
                                , std::int64_t start
                                , bool is_streaming_protocol)
        {
            return std::make_shared<libav_stream_t>(std::move(stream_info)
                                                    , start
                                                    , is_streaming_protocol);
        }

        libav_stream_t(stream_info_t&& stream_info
                       , std::int64_t start
                       , bool is_streaming_protocol)
            : stream_info(std::move(stream_info))
            , delay_controller(this->stream_info)
            , frame_id(0)
            , start(start)
            , is_streaming_protocol(is_streaming_protocol)
        {

        }

        libav_stream_t(libav_stream_t&& other) = default;

        void push_data(frame_t&& frame)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            while (frame_queue.size() >= max_queue_size)
            {
                frame_queue.pop();
            }

            frame_queue.emplace(std::move(frame));
        }

        bool fetch_frame(frame_t&& frame)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            if (!frame_queue.empty())
            {
                frame = std::move(frame_queue.front());
                frame_queue.pop();

                return true;
            }

            return false;
        }

        frame_queue_t fetch_queue()
        {
            std::lock_guard<std::mutex> lock(queue_mutex);

            return std::move(frame_queue);
        }
    };

    struct frame_manager_t
    {
        using frame_pair_t = std::pair<frame_t, libav_stream_t::pointer_t>;
        using frame_pair_queue_t = std::queue<frame_pair_t>;
        using frame_queue_map_t = std::multimap<std::int64_t, frame_pair_t>;
        using session_queue_t = std::map<std::int32_t, frame_queue_map_t>;

        mutable std::mutex  m_mutex;      

        std::size_t         m_min_queue_size;
        std::size_t         m_max_queue_size;

        frame_queue_map_t   m_frames;

        bool                m_has_read;
        bool                m_streaming_protocol;

        frame_manager_t(std::size_t min_queue_size
                        , std::size_t max_queue_size)
            : m_min_queue_size(min_queue_size)
            , m_max_queue_size(max_queue_size)
            , m_has_read(false)
            , m_streaming_protocol(false)
        {

        }


        bool push_frame(frame_pair_t&& frame
                        , std::int64_t frame_order = AV_NOPTS_VALUE)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_frames.size() < m_max_queue_size)
            {
                m_frames.emplace(frame_order
                                 , std::move(frame));
                m_has_read |= m_frames.size() >= m_min_queue_size;
                return true;
            }
            return false;
        }

        bool pop_frame(frame_pair_t& frame)
        {
            std::int64_t pts = AV_NOPTS_VALUE;
            return pop_frame(frame, pts);
        }

        bool pop_frame(frame_pair_t& frame, std::int64_t& frame_order)
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            if (m_has_read || m_streaming_protocol)
            {
                auto it = m_frames.begin();
                if (it != m_frames.end())
                {
                    frame = std::move(it->second);
                    frame_order = it->first;
                    m_frames.erase(it);
                    return true;
                }

                if (!m_streaming_protocol)
                {
                    m_min_queue_size = std::min(m_max_queue_size - 10, m_min_queue_size + m_max_queue_size / 10);
                }
                m_has_read = false;
            }

            return false;
        }

        std::size_t overload_frames() const
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_frames.size() > m_min_queue_size
                    ? m_frames.size() - m_min_queue_size
                    : 0;
        }

        bool is_empty()
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_frames.empty();
        }
    };

    //using stream_map_t = std::map<std::int32_t,libav_stream_t::pointer_t>;
    using stream_collection_t = std::vector<libav_stream_t::pointer_t>;

    mutable std::mutex                                  m_mutex;
    libav_grabber_config_t                              m_config;
    frame_handler_t                                     m_frame_handler;
    stream_event_handler_t                              m_stream_event_handler;

    std::thread                                         m_stream_thread;
    stream_collection_t                                 m_streams;
    std::unique_ptr<libav_input_format_context_t>       m_format_context;

    std::atomic_bool                                    m_is_running;
    capture_diagnostic_t                                m_diagnostic;

    std::uint32_t                                       m_grabber_id;
    std::int64_t                                        m_start_time;

    libav_stream_grabber_context_t(const libav_grabber_config_t& config
                                    , frame_handler_t frame_handler
                                    , stream_event_handler_t stream_event_handler)
        : m_config(config)
        , m_frame_handler(frame_handler)
        , m_stream_event_handler(stream_event_handler)
        , m_format_context(nullptr)
        , m_is_running(false)
        , m_start_time(av_gettime_relative())
    {
        static std::uint32_t cap_id = 0;
        m_grabber_id = cap_id++;

        LOG_T << "grabber #" << m_grabber_id << ". Create for uri " << m_config.url LOG_END;

        m_is_running = true;
        m_stream_thread = std::thread(&libav_stream_grabber_context_t::streamig_proc
                                      , this);
    }

    ~libav_stream_grabber_context_t()
    {
        LOG_T << "grabber #" << m_grabber_id << ". Destroy" LOG_END;
        stop();
    }


    bool open()
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        if (m_format_context == nullptr)
        {
            m_format_context.reset();
            m_format_context.reset(new libav_input_format_context_t(m_is_running));
            m_diagnostic.reconnections++;

            if (m_format_context->init(m_config.url, m_config.options) >= 0)
            {
                m_diagnostic.alive_time = 0;
                m_streams.clear();

                for (auto& s_info : m_format_context->get_streams(m_config.stream_mask))
                {

                    LOG_D << "grabber #" << m_grabber_id << ". Add stream "
                          << s_info.to_string() LOG_END;

                    m_streams.emplace_back(libav_stream_t::create(std::move(s_info)
                                                                  , m_format_context->context->streams[s_info.stream_id]->start_time
                                                                  , m_format_context->is_streaming_protocol));
                }

                push_event(streaming_event_t::open);
            }
            else
            {
                m_format_context.reset(nullptr);
                LOG_E << "grabber #" << m_grabber_id << ". Can't initialize context" LOG_END;
            }
        }

        return is_open();
    }

    bool is_empty() const
    {
        return m_streams.empty();
    }

    bool is_open() const
    {
        return m_format_context != nullptr && !is_empty();
    }

    bool close()
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        if (m_format_context != nullptr)
        {
            m_format_context.reset(nullptr);
            m_streams.clear();
            push_event(streaming_event_t::close);

            return true;
        }

        return false;
    }

    libav_stream_t::pointer_t get_stream(std::int32_t index)
    {
        return static_cast<std::size_t>(index) < m_streams.size()
                ? m_streams[index]
                : nullptr;
    }

    void streamig_proc()
    {

        LOG_I << "grabber #" << m_grabber_id << ". Started" LOG_END;

        push_event(streaming_event_t::start);

        std::uint32_t error_counter = 0;

        auto begin_tp = adaptive_timer_t::now();
        std::uint64_t alive_tp = 0;

        frame_t frame = {};

        frame_manager_t frame_manager(min_queue_size, max_queue_size);

        std::int64_t frame_order = 0;

        std::thread process_thread([&] { frame_processor(frame_manager); });

        while(m_is_running.load(std::memory_order_consume))
        {        
            auto now_time = adaptive_timer_t::now();
            m_diagnostic.total_time = now_time - begin_tp;
            std::uint32_t idle_time = reconnect_timeout_ms;

            if (!is_open())
            {
                if (open())
                {
                    frame_manager.m_streaming_protocol = m_format_context->is_streaming_protocol;
                    frame_manager.m_min_queue_size = min_queue_size;
                }
            }

            if (is_open())
            {                              
                idle_time = 0;

                auto result = m_format_context->fetch_media_data(frame
                                                                 , frame_order);

                if (!m_is_running.load(std::memory_order_consume))
                {
                    break;
                }

                if (result >= 0)
                {
                    if (m_format_context->total_read_frames == 1)
                    {
                        alive_tp = now_time;
                        m_diagnostic.alive_time = 0;
                    }
                    else
                    {
                        m_diagnostic.alive_time = now_time - alive_tp;
                    }

                    error_counter = 0;

                    LOG_T << "grabber #" << m_grabber_id << ". Fetch frame #" << frame.info.stream_id
                              << "(" << frame.info.to_string()
                              << "): size: " << frame.media_data.size() << ", ts:" << frame_order LOG_END;

                    if (auto stream = get_stream(result))
                    {
                        frame.info.media_info = stream->stream_info.media_info;
                        frame.info.codec_id = stream->stream_info.codec_info.id;
                        frame.info.stream_id = stream->stream_info.stream_id;

                        frame_manager_t::frame_pair_t frame_pair(std::move(frame)
                                                                 , stream);

                        do
                        {
                            if (frame_manager.push_frame(std::move(frame_pair)
                                                         , frame_order))
                            {

                                /*
                                std::cout << "Push frame #" << frame.info.id << ": s: "
                                          << stream->stream_info.stream_id << ", ts: "
                                          << frame_order << ", frame ts: " << frame.info.timestamp() << std::endl;
                                */
                                break;
                            }
                            std::this_thread::sleep_for(std::chrono::milliseconds(idle_timeout_ms));
                        }
                        while(m_is_running.load(std::memory_order_consume));
                    }
                }
                else
                {
                    m_diagnostic.errors++;

                    if (result == AVERROR(EAGAIN))
                    {
                        result = 0;
                        error_counter++;
                        idle_time = idle_timeout_ms;
                    }
                }

                if (result < 0)
                {
                    error_counter++;
                    idle_time = idle_timeout_ms;
                    LOG_E << "grabber #" << m_grabber_id << ". Error fetch media data: err #" << result
                          << ": " << utils::error_string(result) LOG_END;
                }

                if (error_counter > max_repetitive_errors
                        || result == AVERROR_EOF)
                {
                    close();
                }
            }

            if (m_is_running == true
                    && idle_time > 0)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(idle_time));
            }
        }

        close();

        if (process_thread.joinable())
        {
            process_thread.join();
        }

        LOG_I << "grabber #" << m_grabber_id << ". Stopped" LOG_END;
        push_event(streaming_event_t::stop);
    }

    void frame_processor(frame_manager_t& frame_manager)
    {
        while(m_is_running.load())
        {
            frame_manager_t::frame_pair_t frame;
            std::int64_t frame_order;
            if (frame_manager.pop_frame(frame, frame_order))
            {
                if (frame.second != nullptr)
                {
                    libav_stream_t& stream = *frame.second;

                    LOG_T << "grabber #" << m_grabber_id << ". Pop frame #" << frame.first.info.stream_id
                              << "(" << frame.first.info.to_string()
                              << "): size: " << frame.first.media_data.size() << ", ts:" << frame_order LOG_END;

                    /*
                    std::cout << "Pop frame #" << frame.first.info.id << ": s: "
                              << stream.stream_info.stream_id << ", ts: "
                              << frame_order << ", frame ts: " << frame.first.info.timestamp() << std::endl;*/

                    if (m_frame_handler == nullptr
                            || !m_frame_handler(stream.stream_info
                                                , std::move(frame.first))
                            )
                    {
                        stream.push_data(std::move(frame.first));
                    }

                    if (stream.stream_info.media_info.media_type == media_type_t::video)
                    {
                        auto overload_frames = frame_manager.overload_frames();
                        if (stream.is_streaming_protocol
                                && overload_frames > 100)
                        {
                            stream.delay_controller.reset();
                        }

                        stream.delay_controller.wait_pts(frame.first.info.timestamp());
                    }
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(idle_timeout_ms));
            }
        }
    }

    void stop()
    {
        if (m_is_running)
        {
            LOG_I << "grabber #" << m_grabber_id << ". Stopping..." LOG_END;

            m_is_running = false;

            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
            }
        }
    }

    frame_queue_t fetch_frame_queue(int32_t stream_id)
    {
        std::lock_guard<std::mutex> lg(m_mutex);

        frame_queue_t frame_queue;

        if (auto stream = get_stream(stream_id))
        {
            frame_queue = std::move(stream->fetch_queue());
        }

        return frame_queue;
    }

    void push_event(streaming_event_t capture_event)
    {
        if (m_stream_event_handler != nullptr)
        {
            m_stream_event_handler(capture_event);
        }
    }
};
//------------------------------------------------------------------------------------
libav_grabber_config_t::libav_grabber_config_t(const std::string &url
                                               , stream_mask_t stream_mask
                                               , std::string options)
    : url(url)
    , stream_mask(stream_mask)
    , options(options)
{

}

//------------------------------------------------------------------------------------
libav_stream_grabber::libav_stream_grabber(frame_handler_t frame_handler
                                           , stream_event_handler_t stream_event_handler)
    : m_frame_handler(frame_handler)
    , m_stream_event_handler(stream_event_handler)
{

}

bool libav_stream_grabber::open(const libav_grabber_config_t &config)
{
    m_libav_stream_grabber_context.reset();
    m_libav_stream_grabber_context.reset(new libav_stream_grabber_context_t(config
                                              , m_frame_handler
                                              , m_stream_event_handler)
                          );

    return true;
}

bool libav_stream_grabber::close()
{
    m_libav_stream_grabber_context.reset();

    return true;
}

bool libav_stream_grabber::is_opened() const
{
    return m_libav_stream_grabber_context != nullptr;
}

bool libav_stream_grabber::is_established() const
{
    return m_libav_stream_grabber_context != nullptr
            && m_libav_stream_grabber_context->is_open();
}

bool libav_stream_grabber::get_config(libav_grabber_config_t &config)
{
    if (m_libav_stream_grabber_context != nullptr)
    {
        config = m_libav_stream_grabber_context->m_config;
        return true;
    }

    return false;
}

bool libav_stream_grabber::set_config(const libav_grabber_config_t &config)
{
    if (m_libav_stream_grabber_context != nullptr)
    {
        m_libav_stream_grabber_context->m_config = config;
        return true;
    }

    return false;
}

capture_diagnostic_t libav_stream_grabber::diagnostic() const
{
    capture_diagnostic_t state;

    if (m_libav_stream_grabber_context != nullptr)
    {
        state = m_libav_stream_grabber_context->m_diagnostic;
    }

    return state;
}

stream_info_list_t libav_stream_grabber::streams() const
{
    stream_info_list_t stream_info_list;

    if (m_libav_stream_grabber_context != nullptr)
    {
        for (const auto& stream : m_libav_stream_grabber_context->m_streams)
        {
            stream_info_list.emplace_back(stream->stream_info);
        }
    }

    return stream_info_list;
}

frame_queue_t libav_stream_grabber::fetch_media_queue(int32_t stream_id)
{
    frame_queue_t frame_queue;

    if (m_libav_stream_grabber_context != nullptr)
    {
        frame_queue = std::move(m_libav_stream_grabber_context->fetch_frame_queue(stream_id));
    }

    return frame_queue;
}
//------------------------------------------------------------------------------------

}
