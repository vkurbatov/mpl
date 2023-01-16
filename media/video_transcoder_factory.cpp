#include "video_transcoder_factory.h"
#include "video_format_impl.h"

#include "core/convert_utils.h"

#include "i_message_frame.h"
#include "i_video_frame.h"

#include "core/message_event_impl.h"
#include "core/event_channel_state.h"

#include "core/message_sink_impl.h"
#include "core/message_router_impl.h"


#include "tools/ffmpeg/libav_converter.h"
#include "tools/ffmpeg/libav_transcoder.h"

namespace mpl::media
{

class libav_transcoder
{

};

class video_transcoder : public i_media_transcoder
{
    video_format_impl       m_input_format;
    video_format_impl       m_output_format;

    message_sink_impl       m_sink;
    message_router_impl     m_source_router;

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
    {

    }

    ~video_transcoder()
    {

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
        switch(message.category())
        {
            case message_category_t::frame:
                return on_sink_frame_message(static_cast<const i_message_frame&>(message));
            break;
            default:;
        }

        return false;
    }

    // i_media_transcoder interface
public:

    const i_media_format &format() const override
    {
        return m_output_format;
    }

    // i_message_source interface
public:
    bool add_sink(i_message_sink *sink) override
    {
        return m_source_router.add_sink(sink);
    }

    bool remove_sink(i_message_sink *sink) override
    {
        return m_source_router.remove_sink(sink);
    }

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override
    {
        return on_sink_message(message);
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
