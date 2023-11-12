#include "libav_input_device_factory.h"

#include "utils/message_router_impl.h"
#include "utils/property_writer.h"
#include "utils/message_event_impl.h"
#include "utils/time_utils.h"
#include "utils/adaptive_delay.h"
#include "utils/endian_utils.h"
#include "core/event_channel_state.h"

#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"

#include "tools/utils/sync_base.h"
#include "tools/ffmpeg/libav_input_format.h"
#include "tools/ffmpeg/libav_utils.h"
#include "tools/codec/h264/h264_utils.h"

#include <thread>
#include <shared_mutex>
#include <atomic>
#include <map>
#include <iostream>


namespace mpl::media
{

namespace detail
{


smart_buffer create_buffer(const pt::ffmpeg::frame_ref_t& libav_frame
                           , const pt::ffmpeg::stream_info_t& stream)
{
    if (stream.codec_info.id == pt::ffmpeg::codec_id_h264)
    {
        if (stream.extra_data != nullptr)
        {
            auto fragmentation_type = pt::codec::get_fragmentation_type(stream.extra_data->data()
                                                                        , stream.extra_data->size());
            switch(fragmentation_type)
            {
                case pt::codec::h264_fragmentation_type_t::avcc_8:
                case pt::codec::h264_fragmentation_type_t::avcc_16:
                case pt::codec::h264_fragmentation_type_t::avcc_24:
                case pt::codec::h264_fragmentation_type_t::avcc_32:
                {
                    auto fragments = pt::codec::split_fragments(fragmentation_type
                                                                , libav_frame.data
                                                                , libav_frame.size);
                    smart_buffer new_buffer;
                    for (const auto& f : fragments)
                    {
                        new_buffer.append_data(&pt::codec::annex_b_start_code, sizeof(pt::codec::annex_b_start_code));
                        new_buffer.append_data(static_cast<const std::uint8_t*>(libav_frame.data) + f.payload_offset
                                               , f.payload_length);
                    }

                    if (!new_buffer.is_empty())
                    {
                        return new_buffer;
                    }
                }
                break;
                default:;
            }
        }
    }

    return smart_buffer(libav_frame.data
                        , libav_frame.size);
}

}

struct stream_t
{
    static constexpr timestamp_t min_overtime = durations::seconds(1);
    static constexpr timestamp_t max_overtime = durations::seconds(3);

    using array_t = std::vector<stream_t>;

    pt::ffmpeg::stream_info_t   stream_info;
    timestamp_t                 start_timestamp;
    frame_id_t                  frame_id;
    timestamp_t                 last_timestamp;
    timestamp_t                 ntp_timestamp;
    timestamp_t                 current_timestamp;
    timestamp_t                 overtime;

    inline static timestamp_t now()
    {
        return utils::time::get_ticks();
    }

    inline static stream_t::array_t create_list(pt::ffmpeg::stream_info_t::list_t&& streams)
    {
        stream_t::array_t stream_list;

        for (auto && s : streams)
        {
            stream_list.emplace_back(std::move(s));
        }

        return stream_list;
    }

    stream_t(pt::ffmpeg::stream_info_t&& stream_info)
        : stream_info(std::move(stream_info))
        , start_timestamp(0)
        , frame_id(0)
        , last_timestamp(0)
        , ntp_timestamp(now())
        , current_timestamp(0)
        , overtime(max_overtime)
    {
        // reset();
    }

    inline void reset()
    {
        start_timestamp = 0;
        frame_id = 0;
        last_timestamp = 0;
        ntp_timestamp = now();
        current_timestamp = 0;
        overtime = min_overtime;
    }

    void next()
    {
        frame_id++;
    }

    timestamp_t push_timestamp(timestamp_t native_timestamp)
    {
        auto dt = native_timestamp - last_timestamp;
        if (frame_id == 0
                || native_timestamp < start_timestamp
                || dt < 0
                || dt > stream_info.media_info.sample_rate())
        {
            dt = 0;
        }

        last_timestamp = native_timestamp;

        current_timestamp += dt;

        return current_timestamp;
    }

    void sync() const
    {
        auto rdt = now() - ntp_timestamp;
        auto pdt = durations::milliseconds(current_timestamp * 1000) / stream_info.media_info.sample_rate();

        auto dt = pdt - rdt;
        if (dt > overtime)
        {
            utils::time::sleep(dt - overtime);
        }
    }
    timestamp_t get_ntp_timestamp() const
    {
        return ntp_timestamp + durations::milliseconds(current_timestamp * 1000) / stream_info.media_info.sample_rate();
    }
};

class libav_input_device : public i_device
{
    using mutex_t = pt::utils::shared_spin_lock;
    using lock_t = std::lock_guard<mutex_t>;
    using shared_lock_t = std::shared_lock<mutex_t>;

    struct device_params_t
    {
        device_type_t   device_type;
        std::string     url;
        std::string     options;

        device_params_t(device_type_t device_type = device_type_t::libav_in
                , const std::string_view& url = {}
                , const std::string_view& options = {})
            : device_type(device_type)
            , url(url)
            , options(options)
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
            return reader.get("url", url)
                    | reader.get("options", options);
        }

        bool save(i_property& params) const
        {
            property_writer writer(params);
            return writer.set("url", url)
                    && writer.set("options", options);
        }

        bool is_valid() const
        {
            return device_type == device_type_t::libav_in
                    && !url.empty();
        }

        pt::ffmpeg::libav_input_format::config_t native_config() const
        {
            return { url
                     , options };
        }
    };

    device_params_t                 m_device_params;
    message_router_impl             m_router;

    frame_id_t                      m_frame_counter;
    timestamp_t                     m_frame_timestamp;


    timestamp_t                     m_start_time;

    std::thread                     m_thread;

    channel_state_t                 m_state;
    std::atomic_bool                m_open;

public:
    using u_ptr_t = std::unique_ptr<libav_input_device>;
    using s_ptr_t = std::shared_ptr<libav_input_device>;

    static u_ptr_t create(const i_property &device_params)
    {
        device_params_t libav_params(device_params);
        if (libav_params.is_valid())
        {
            return std::make_unique<libav_input_device>(std::move(libav_params));
        }

        return nullptr;
    }

    libav_input_device(device_params_t&& device_params)
        : m_device_params(std::move(device_params))
        , m_frame_counter(0)
        , m_frame_timestamp(0)
        , m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    ~libav_input_device()
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
            m_thread = std::thread([&]{ grabbing_thread(); });
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

    void grabbing_thread()
    {
        change_state(channel_state_t::open);

        std::size_t error_counter = 0;

        while(is_open())
        {
            pt::ffmpeg::libav_input_format native_input_device(m_device_params.native_config());

            change_state(channel_state_t::connecting);

            if (native_input_device.open())
            {
                auto streams = stream_t::create_list(native_input_device.streams());
                change_state(channel_state_t::connected);
                error_counter = 0;

                while(is_open()
                      && error_counter < 10)
                {
                    pt::ffmpeg::frame_ref_t libav_frame;
                    if (native_input_device.read(libav_frame))
                    {
                        auto& stream = streams[libav_frame.info.stream_id];
                        error_counter = 0;
                        on_native_frame(stream
                                        , libav_frame);

                        if (libav_frame.info.stream_id == 0)
                        {
                            stream.sync();
                        }
                        stream.next();
                    }
                    else
                    {
                        error_counter++;
                        if (is_open())
                        {
                            utils::time::sleep(durations::milliseconds(100));
                        }
                    }
                }

                change_state(channel_state_t::disconnecting);
                native_input_device.close();
                change_state(channel_state_t::disconnected);
            }
        }
    }

    void reset()
    {
        m_frame_counter = 0;
        m_frame_timestamp = 0;
        m_start_time = utils::time::now();
    }

    timestamp_t elapsed_time() const
    {
        return utils::time::now() - m_start_time;
    }


    bool on_native_frame(stream_t& stream
                         , pt::ffmpeg::frame_ref_t& libav_frame)
    {
        if (libav_frame.size > 0)
        {
            switch(stream.stream_info.media_info.media_type)
            {
                case pt::ffmpeg::media_type_t::audio:
                {
                    audio_format_impl format;
                    if (utils::convert(stream.stream_info
                                       , format))
                    {
                        audio_frame_impl frame(format
                                               , stream.frame_id
                                               , stream.push_timestamp(libav_frame.info.timestamp()));

                        frame.set_ntp_timestamp(stream.get_ntp_timestamp());
                        frame.set_stream_id(stream.stream_info.program_id);
                        frame.set_track_id(stream.stream_info.stream_id);

                        frame.smart_buffers().set_buffer(media_buffer_index
                                                         , smart_buffer(libav_frame.data
                                                                        , libav_frame.size));

                        return m_router.send_message(frame);
                    }
                }
                break;
                case pt::ffmpeg::media_type_t::video:
                {
                    video_format_impl format;
                    if (utils::convert(stream.stream_info
                                       , format))
                    {
                        video_frame_type_t frame_type = format.is_convertable()
                                ? video_frame_type_t::undefined
                                : video_frame_type_t::delta_frame;

                        if (libav_frame.info.key_frame)
                        {
                            frame_type = video_frame_type_t::key_frame;
                        }

                        video_frame_impl frame(format
                                               , stream.frame_id
                                               , stream.push_timestamp(libav_frame.info.timestamp())
                                               , frame_type);

                        frame.set_stream_id(stream.stream_info.program_id);
                        frame.set_track_id(stream.stream_info.stream_id);
                        frame.set_ntp_timestamp(stream.get_ntp_timestamp());

                        frame.smart_buffers().set_buffer(media_buffer_index
                                                         , detail::create_buffer(libav_frame
                                                                                 , stream.stream_info));


                        return m_router.send_message(frame);
                    }
                }
                break;
                default:;
            }
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
        return m_open.load(std::memory_order_acquire);
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_device interface
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
    device_type_t device_type() const override
    {
        return device_type_t::v4l2_in;
    }

    // i_parametrizable interface
public:
    bool set_params(const i_property &params) override
    {
        if (!m_open)
        {
            auto device_params = m_device_params;
            if (device_params.load(params)
                    && device_params.is_valid())
            {
                m_device_params = device_params;
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

libav_input_device_factory::u_ptr_t libav_input_device_factory::create()
{
    return std::make_unique<libav_input_device_factory>();
}

libav_input_device_factory::libav_input_device_factory()
{

}

i_device::u_ptr_t libav_input_device_factory::create_device(const i_property &device_params)
{
    return libav_input_device::create(device_params);
}



}
