#include "media_frame_selector.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"
#include "utils/option_helper.h"
#include "media_option_types.h"
#include "utils/pointer_utils.h"
#include "media_message_types.h"

namespace mpl::media
{

namespace detail
{

using frame_info_t = media_frame_selector::frame_info_t;

bool is_track_compatible(const i_option& frame_options
                         , const i_option& format_options)
{
    option_reader frame_reader(frame_options);
    option_reader format_reader(format_options);

    return frame_reader.get<stream_id_t>(opt_frm_stream_id) == format_reader.get<stream_id_t>(opt_frm_stream_id)
            && frame_reader.get<track_id_t>(opt_frm_track_id) == format_reader.get<track_id_t>(opt_frm_track_id);
}


bool is_best_format(const i_audio_format& frame_format
                    , const i_audio_format& current_format)
{
    return (frame_format.sample_rate() > current_format.sample_rate())
            || (frame_format.sample_rate() == current_format.sample_rate()
                && frame_format.channels() > current_format.channels());
}

bool is_best_format(const i_video_format& frame_format
                    , const i_video_format& current_format)
{
    auto frame_size = frame_format.width() * frame_format.height();
    auto current_size = current_format.width() * current_format.height();
    return (frame_size > current_size && frame_format.frame_rate() >= current_format.frame_rate())
            || (frame_size == current_size && frame_format.frame_rate() > current_format.frame_rate());
}

bool select_frame(const i_audio_frame& frame
                  , const frame_info_t& frame_info
                  , const media_frame_selector::selection_handler_t& selection_handler)
{
    if (selection_handler)
    {
        return selection_handler(frame, frame_info);
    }
    else if (!frame_info.is_defined())
    {
        return true;
    }
    else if (track_info_t(frame.options()).is_compatible(frame_info.track_info))
    {
        return true;
    }
    return false;
}

bool select_frame(const i_video_frame& frame
                  , const frame_info_t& frame_info
                  , const media_frame_selector::selection_handler_t& selection_handler)
{
    if (selection_handler)
    {
        return selection_handler(frame, frame_info);
    }
    else if (!frame_info.is_defined())
    {
        if (frame.frame_type() != i_video_frame::frame_type_t::delta_frame)
        {
            return true;
        }
    }
    else if (track_info_t(frame.options()).is_compatible(frame_info.track_info))
    {
        return true;
    }

    return false;
}

}

media_frame_selector::config_t::config_t(timestamp_t active_timeout)
    : active_timeout(active_timeout)
{

}

media_frame_selector::frame_info_t::frame_info_t(const i_media_frame &media_frame)
{
    update(media_frame);
}

void media_frame_selector::frame_info_t::update(const i_media_frame &media_frame)
{
    switch(media_frame.media_type())
    {
        case media_type_t::audio:
            media_format = utils::static_pointer_cast<i_media_format>(static_cast<const i_audio_frame&>(media_frame).format().clone());
        break;
        case media_type_t::video:
            media_format = utils::static_pointer_cast<i_media_format>(static_cast<const i_video_frame&>(media_frame).format().clone());
        break;
        default:
            return;
    }

    track_info.load(media_frame.options());
}

void media_frame_selector::frame_info_t::reset()
{
    *this = {};
}

bool media_frame_selector::frame_info_t::is_defined() const
{
    return media_format != nullptr;
}

media_frame_selector::media_frame_selector(const config_t &config
                                           , i_message_sink *output_sink)
    : m_config(config)
    , m_output_sink(output_sink)
    , m_selection_handler(nullptr)
{

}

void media_frame_selector::set_sink(i_message_sink *output_sink)
{
    m_output_sink = output_sink;
}

void media_frame_selector::reset()
{
    m_audio_track_info.reset();
    m_video_track_info.reset();
}

const media_frame_selector::frame_info_t& media_frame_selector::audio_track() const
{
    return m_audio_track_info;
}

const media_frame_selector::frame_info_t& media_frame_selector::video_track() const
{
    return m_video_track_info;
}

void media_frame_selector::set_handler(const selection_handler_t &handler)
{
    m_selection_handler = handler;
}

bool media_frame_selector::send_message(const i_message &message)
{
    if (message.subclass() == message_class_media)
    {
        switch(message.category())
        {
            case message_category_t::data:
            {
                auto& media_frame = static_cast<const i_media_frame&>(message);
                if (select_frame(media_frame))
                {
                    return send_frame(media_frame);
                }
            }
            break;
            default:;
        }

    }

    return false;
}

bool media_frame_selector::send_frame(const i_media_frame &frame)
{
    return m_output_sink != nullptr
            && m_output_sink->send_message(frame);
}

bool media_frame_selector::select_frame(const i_media_frame &frame)
{
    switch(frame.media_type())
    {
        case media_type_t::audio:
        {
            auto& audio_frame = static_cast<const i_audio_frame&>(frame);
            if (detail::select_frame(audio_frame
                                     , m_audio_track_info
                                     , m_selection_handler))
            {
                if (!m_audio_track_info.is_defined()
                        || !audio_frame.format().is_compatible(*m_audio_track_info.media_format))
                {
                    m_audio_track_info.update(frame);
                }

                return true;
            }
        }
        break;
        case media_type_t::video:
        {
            auto& video_frame = static_cast<const i_video_frame&>(frame);
            if (detail::select_frame(video_frame
                                     , m_video_track_info
                                     , m_selection_handler))
            {
                if (!m_video_track_info.is_defined()
                        || !video_frame.format().is_compatible(*m_video_track_info.media_format))
                {
                    m_video_track_info.update(frame);
                }

                return true;
            }
        }
        break;
        default:;
    }

    return false;
}


}
