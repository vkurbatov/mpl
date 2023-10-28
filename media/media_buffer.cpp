#include "media_buffer.h"
#include "utils/pointer_utils.h"
#include "utils/enum_utils.h"
#include "media_types.h"
#include "media_message_types.h"

#include <iostream>


namespace mpl::media
{

media_buffer::config_t::config_t(timestamp_t sync_duration)
    : sync_duration(sync_duration)
{

}

bool media_buffer::config_t::is_transit() const
{
    return sync_duration <= timestamp_null;
}

media_buffer::media_buffer(const config_t &config
                           , i_message_sink *output_sink)
    : m_config(config)
    , m_output_sink(output_sink)
{

}

void media_buffer::set_sink(i_message_sink *output_sink)
{
    m_output_sink = output_sink;
}

std::size_t media_buffer::pending_frames() const
{
    return m_frames.size();
}

timestamp_t media_buffer::delay() const
{
    if (!m_frames.empty())
    {
        return m_frames.rbegin()->first - m_frames.begin()->first;
    }

    return timestamp_null;
}

std::size_t media_buffer::process()
{
    std::size_t result = 0;

    while(auto frame = fetch_frame())
    {
        send_frame(*frame);
        result++;
    }

    return result;
}

void media_buffer::reset()
{
    m_frames.clear();
}

bool media_buffer::send_message(const i_message &message)
{
    if (message.subclass() == message_class_media)
    {
        switch(message.category())
        {
            case message_category_t::data:
            {
                if (m_config.is_transit())
                {
                    return send_frame(static_cast<const i_media_frame&>(message));
                }
                if (i_media_frame::u_ptr_t frame = utils::static_pointer_cast<i_media_frame>(message.clone()))
                {
                    return push_frame(frame->ntp_timestamp()
                                      , std::move(frame));
                }
            }
            break;
            default:;
        }

    }

    return false;
}

bool media_buffer::push_frame(timestamp_t timestamp
                              , i_media_frame::u_ptr_t &&frame)
{
    m_frames.emplace(timestamp
                     , std::move(frame));
    process();
    return true;
}

bool media_buffer::send_frame(const i_media_frame &frame)
{
    return m_output_sink != nullptr
            && m_output_sink->send_message(frame);
}

i_media_frame::u_ptr_t media_buffer::fetch_frame()
{
    if (delay() >= m_config.sync_duration)
    {
        auto frame = std::move(m_frames.begin()->second);
        m_frames.erase(m_frames.begin());
        return frame;
    }
    return nullptr;
}

}
