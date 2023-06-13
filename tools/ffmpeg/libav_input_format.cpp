#include "libav_input_format.h"
#include "libav_utils.h"

extern "C"
{
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
}

#include "tools/base/url_base.h"

#include <iostream>

namespace ffmpeg
{

namespace detail
{

}

using config_t = libav_input_format::config_t;
constexpr std::uint64_t read_timeout_ms = 5000;

struct interrupt_timeout_t
{
    std::uint64_t           end_timestamp;
    bool                    active;

    interrupt_timeout_t(bool active = true)
        : end_timestamp(0)
        , active(active)
    {

    }

    void reset(std::uint64_t timeout = read_timeout_ms)
    {
        end_timestamp = adaptive_timer_t::now() + timeout;
        active = true;
    }

    bool is_timeout() const
    {
        return end_timestamp < adaptive_timer_t::now();
    }

    bool is_interrupt() const
    {
        return !active || is_timeout();
    }

    void cancel()
    {
        active = false;
    }

    static std::int32_t check_interrupt(void* timeout_context)
    {
        return timeout_context != nullptr
                && static_cast<interrupt_timeout_t*>(timeout_context)->is_interrupt();
    }
};

struct libav_input_format::context_t
{
    struct native_context_t
    {
        using u_ptr_t = std::unique_ptr<native_context_t>;

        struct AVFormatContext*     m_context;
        struct AVPacket             m_packet;
        interrupt_timeout_t         m_interrupt_timeout;
        stream_info_t::list_t       m_streams;
        bool                        m_open;

        static u_ptr_t create()
        {
            if (auto context = avformat_alloc_context())
            {
                return std::make_unique<native_context_t>(context);
            }

            return nullptr;
        }

        native_context_t(AVFormatContext* context)
            : m_context(context)
            , m_open(false)
        {
            av_init_packet(&m_packet);
        }

        ~native_context_t()
        {
            close();
            av_packet_unref(&m_packet);

            if (m_context != nullptr)
            {
                avformat_free_context(m_context);
            }
        }

        bool native_open(const config_t& config)
        {
            AVDictionary* av_options = nullptr;
            AVInputFormat *input_format = nullptr;

            url_format_t url_format = utils::fetch_url_format(config.url);

            if (!url_format.format_type.empty())
            {
                input_format = av_find_input_format(url_format.format_type.c_str());
            }

            utils::set_options(&av_options
                               , config.options);

            m_context->interrupt_callback.opaque = &m_interrupt_timeout;
            m_context->interrupt_callback.callback = &interrupt_timeout_t::check_interrupt;
            m_context->flags |= AVFMT_FLAG_NONBLOCK;

            m_interrupt_timeout.reset();

            auto result = avformat_open_input(&m_context
                                              , url_format.url.c_str()
                                              , input_format
                                              , &av_options);

            av_dict_free(&av_options);

            if (result == 0)
            {
                result = avformat_find_stream_info(m_context
                                                   , nullptr);

                if (result >= 0)
                {
                    m_streams = fetch_streams();
                    return true;
                }

                avformat_close_input(&m_context);
            }

            return false;
        }

        bool open(const config_t& config)
        {
            if (!m_open)
            {
                m_open = native_open(config);
                return m_open;
            }

            return false;
        }

        bool close()
        {
            if (m_open)
            {
                m_open = false;
                avformat_close_input(&m_context);
                return true;
            }

            return false;
        }

        stream_info_list_t fetch_streams()
        {
            stream_info_list_t streams;

            for (std::uint32_t i = 0; i < m_context->nb_streams; i++)
            {
                stream_info_t stream;
                stream << *m_context->streams[i];
                streams.emplace_back(stream);
            }

            return streams;
        }

        void seek(std::int64_t timestamp)
        {
            for (std::uint32_t i = 0; i < m_context->nb_streams; i++)
            {
                av_seek_frame(m_context
                              , i
                              , timestamp
                              , 0);
            }

        }

        std::int32_t read_frame(frame_ref_t& frame_ref)
        {
            std::int32_t result = -1;

            if (m_open)
            {
                m_interrupt_timeout.reset();
                av_packet_unref(&m_packet);
                result = av_read_frame(m_context
                                       , &m_packet);

                if (result >= 0
                        && m_packet.size > 0)
                {
                    auto& stream = m_context->streams[m_packet.stream_index];

                    switch(stream->codecpar->codec_type)
                    {
                        case AVMEDIA_TYPE_AUDIO:
                            av_packet_rescale_ts(&m_packet, stream->time_base, { 1, stream->codecpar->sample_rate });
                        break;
                        case AVMEDIA_TYPE_VIDEO:
                            av_packet_rescale_ts(&m_packet, stream->time_base, { 1, video_sample_rate });
                        break;
                    }

                    frame_ref.info.dts = m_packet.dts;
                    frame_ref.info.pts = m_packet.pts;
                    frame_ref.info.stream_id = m_packet.stream_index;
                    frame_ref.info.key_frame = (m_packet.flags & AV_PKT_FLAG_KEY) != 0;

                    frame_ref.info.codec_id = stream->codecpar->codec_id;
                    frame_ref.info.media_info = m_streams[m_packet.stream_index].media_info;

                    frame_ref.data = m_packet.data;
                    frame_ref.size = m_packet.size;

                    result = m_packet.stream_index;
                }
                else
                {
                    switch(result)
                    {
                        case AVERROR_EOF:
                            seek(0);
                        break;
                        default:;
                    }
                }
            }

            return result;
        }

        std::int32_t read_frame(frame_t& frame)
        {
            frame_ref_t frame_ref;
            auto result = read_frame(frame_ref);
            if (result >= 0)
            {
                frame.info = std::move(frame_ref.info);
                frame.media_data = frame_ref.get_media_data();
            }

            return result;
        }

        void cancel()
        {
            m_interrupt_timeout.cancel();
        }

        const stream_info_t::list_t& streams() const
        {
            return m_streams;
        }
    };


    using u_ptr_t = std::unique_ptr<context_t>;

    config_t                    m_config;
    frame_handler_t             m_frame_handler;
    native_context_t::u_ptr_t   m_native_context;

    static u_ptr_t create(const libav_input_format::config_t& config)
    {
        return std::make_unique<context_t>(config);
    }

    context_t(const libav_input_format::config_t& config)
        : m_config(config)
    {

    }

    ~context_t()
    {
        close();
    }

    const config_t& config() const
    {
        return m_config;
    }

    bool set_config(const config_t& config)
    {
        if (!is_open())
        {
            m_config = config;
            return true;
        }

        return false;
    }

    stream_info_list_t streams() const
    {
        if (m_native_context)
        {
            return m_native_context->streams();
        }
        return {};
    }

    bool open()
    {
        if (m_native_context == nullptr)
        {
            m_native_context = native_context_t::create();
            if (m_native_context != nullptr)
            {
                if (m_native_context->open(m_config))
                {
                    return true;
                }

                close();
            }
        }

        return false;
    }

    bool close()
    {
        if (m_native_context != nullptr)
        {
            m_native_context.reset();
            return true;
        }

        return false;
    }

    bool is_open() const
    {
        return m_native_context != nullptr;
    }

    bool cancel()
    {
        return false;
    }

    void seek(timestamp_t timestamp)
    {
        if (m_native_context != nullptr)
        {
            m_native_context->seek(timestamp);
        }
    }

    bool read(frame_t& frame)
    {
        if (m_native_context != nullptr)
        {
            return m_native_context->read_frame(frame) >= 0;
        }
        return false;
    }

    bool read(frame_ref_t& frame_frame)
    {
        if (m_native_context != nullptr)
        {
            return m_native_context->read_frame(frame_frame) >= 0;
        }
        return false;
    }

};

libav_input_format::config_t::config_t(const std::string_view &url
                                       , const std::string_view &options)
    : url(url)
    , options(options)
{

}

bool libav_input_format::config_t::is_valid() const
{
    return !url.empty();
}

libav_input_format::libav_input_format(const config_t &config)
    : m_context(context_t::create(config))
{

}

libav_input_format::~libav_input_format()
{

}

const libav_input_format::config_t &libav_input_format::config() const
{
    return m_context->config();
}

bool libav_input_format::set_config(const config_t &config)
{
    return m_context->set_config(config);
}


stream_info_list_t libav_input_format::streams() const
{
    return m_context->streams();
}

bool libav_input_format::open()
{
    return m_context->open();
}

bool libav_input_format::close()
{
    return m_context->close();
}

bool libav_input_format::is_open() const
{
    return m_context->is_open();
}

bool libav_input_format::cancel()
{
    return m_context->cancel();
}

void libav_input_format::seek(timestamp_t timestamp)
{
    m_context->seek(timestamp);
}

bool libav_input_format::read(frame_t &frame)
{
    return m_context->read(frame);
}

bool libav_input_format::read(frame_ref_t &frame_ref)
{
    return m_context->read(frame_ref);
}


}
