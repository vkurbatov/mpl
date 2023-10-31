#include "track_metrics_manager.h"
#include "i_audio_frame.h"
#include "i_video_frame.h"
#include "core/i_buffer_collection.h"
#include "media_types.h"
#include "media_message_types.h"

#include "utils/time_utils.h"


namespace mpl::media
{

track_metrics_manager::track_t::track_t(const track_info_t &track_info)
{
    metrics.track_info = track_info;
    metrics.timestamp = utils::time::get_ticks();
}

void track_metrics_manager::track_t::push_frame(const i_media_frame &media_frame)
{
    if (auto buffer = media_frame.data().get_buffer(media_buffer_index))
    {
        bitrate_calculator.push_frame(buffer->size());
        metrics.flow_stats = bitrate_calculator.stats();
        metrics.timestamp = utils::time::get_ticks();
        switch(media_frame.media_type())
        {
            case media_type_t::audio:
            {
                auto& audio_frame = static_cast<const i_audio_frame&>(media_frame);
                metrics.format = audio_info_t(audio_frame.format());
            }
            break;
            case media_type_t::video:
            {
                auto& video_frame = static_cast<const i_video_frame&>(media_frame);
                metrics.format = video_info_t(video_frame.format());
            }
            break;
            default:;
        }
    }
}

void track_metrics_manager::track_t::reset()
{
    bitrate_calculator.reset();
    metrics.flow_stats.reset();
    metrics.format = {};
}

track_metrics_manager::track_metrics_manager(const config_t &config)
    : m_config(config)
    , m_check_timer(m_config.check_interval)
{

}

track_metrics_t::array_t track_metrics_manager::get_metrics() const
{
    track_metrics_t::array_t metrics;
    for (const auto& [t, m] : m_tracks)
    {
        metrics.push_back(m.metrics);
    }

    return metrics;
}

void track_metrics_manager::reset()
{
    m_tracks.clear();
}

void track_metrics_manager::push_message(const i_message &message)
{
    if (message.category() == message_category_t::data
            && message.subclass() == message_class_media)
    {
        auto& media_frame = static_cast<const i_media_frame&>(message);
        if (auto track = query_track(media_frame))
        {
            track->push_frame(media_frame);
        }
    }

    remove_garbage();
}

std::size_t track_metrics_manager::remove_garbage()
{
    std::size_t result = 0;
    if (m_check_timer.check_timeout())
    {
        auto now = utils::time::get_ticks();

        for (auto it = m_tracks.begin(); it != m_tracks.end();)
        {
            auto dt = now - it->second.metrics.timestamp;
            if (dt >= m_config.check_interval)
            {
                it = m_tracks.erase(it);
                result++;
            }
            else
            {
                it = std::next(it);
            }
        }
    }

    return result;
}

track_metrics_manager::track_t *track_metrics_manager::query_track(const i_media_frame &media_frame)
{
    track_info_t frame_info(media_frame.options());
    if (auto it = m_tracks.find(frame_info); it != m_tracks.end())
    {
        return &it->second;
    }

    if (auto it = m_tracks.emplace(frame_info
                                   , frame_info)
            ; it.second)
    {
        return &it.first->second;
    }

    return nullptr;
}

}

