#include "video_transcoder_factory.h"
#include "video_format_impl.h"

#include "i_message_frame.h"
#include "i_video_frame.h"

#include "message_event_impl.h"
#include "event_channel_state.h"

#include "message_sink_impl.h"
#include "message_router_impl.h"

#include "tools/ffmpeg/libav_converter.h"
#include "tools/ffmpeg/libav_transcoder.h"

namespace mpl
{

class video_transcoder : public i_media_transcoder
        , public i_message_sink
{
    video_format_impl       m_input_format;
    video_format_impl       m_output_format;

    message_sink_impl       m_sink;
    message_router_impl     m_source_router;

    channel_state_t         m_state;
    bool                    m_open;

public:
    using u_ptr_t = std::unique_ptr<video_transcoder>;

    static u_ptr_t create(const i_property &property)
    {
        return nullptr;
    }

    video_transcoder(video_format_impl&& input_format
                     , video_format_impl&& output_format)
        : m_input_format(std::move(input_format))
        , m_output_format(std::move(output_format))
        , m_sink([&](const auto& message) { return on_sink_message(message); })
        , m_state(channel_state_t::ready)
        , m_open(false)
    {

    }

    ~video_transcoder()
    {

    }

    void on_change_state(channel_state_t new_state
                         , const std::string_view& reason)
    {
        if (m_state != new_state)
        {
            m_state = new_state;
            m_source_router.send_message(message_event_impl<event_channel_state_t>({new_state, reason}));
        }
    }

    bool on_sink_video_frame(const i_option& options
                             , const i_video_frame& video_frame)
    {
        if (m_input_format.is_compatible(video_frame.format()))
        {

        }

        return false;
    }

    bool on_sink_frame_message(const i_message_frame& message_frame)
    {
        switch(message_frame.frame().media_type())
        {
            case media_type_t::video:
            {
                return on_sink_video_frame(message_frame.options()
                                           , static_cast<const i_video_frame&>(message_frame.frame()));
            }
            break;
            default:;
        }

        return false;
    }

    bool on_sink_message(const i_message& message)
    {
        if (video_transcoder::is_open())
        {
            switch(message.category())
            {
                case message_category_t::frame:
                    return on_sink_frame_message(static_cast<const i_message_frame&>(message));
                break;
                default:;
            }
        }

        return false;
    }

    bool open()
    {
        if (!m_open)
        {

        }

        return false;
    }

    bool close()
    {
        if (m_open)
        {

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
        return m_open;
    }

    channel_state_t state() const override
    {
        return m_state;
    }

    // i_message_channel interface
public:
    i_message_sink *sink() override
    {
        return &m_sink;
    }

    i_message_source *source() override
    {
        return &m_source_router;
    }

    // i_media_transcoder interface
public:
    const i_media_format &input_format() const override
    {
        return m_input_format;
    }

    const i_media_format &output_format() const override
    {
        return m_output_format;
    }
};


video_transcoder_factory::video_transcoder_factory()
{

}

i_media_transcoder::u_ptr_t video_transcoder_factory::create_transcoder(const i_property &property)
{
    return video_transcoder::create(property);
}



}
