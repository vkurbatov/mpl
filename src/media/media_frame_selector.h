#ifndef MPL_MEDIA_FRAME_SELECTOR_H
#define MPL_MEDIA_FRAME_SELECTOR_H

#include "core/i_message_sink.h"
#include "i_media_frame.h"
#include "i_audio_format.h"
#include "i_video_format.h"
#include "track_info.h"
#include <functional>
#include <map>

namespace mpl::media
{

class media_frame_selector : public i_message_sink
{
public:
    struct config_t
    {
        timestamp_t     active_timeout;
        config_t(timestamp_t active_timeout = timestamp_null);
    };

    struct frame_info_t
    {
        i_media_format::u_ptr_t     media_format;
        track_info_t                track_info;

        frame_info_t() = default;
        frame_info_t(const i_media_frame& media_frame);

        void update(const i_media_frame& media_frame);
        void reset();
        bool is_defined() const;
        bool is_compatible(const frame_info_t& other);
    };

    using selection_handler_t = std::function<bool(const i_media_frame&, const frame_info_t& frame_info)>;
private:

    using format_map_t = std::map<track_id_t, i_media_frame::u_ptr_t>;

    config_t                    m_config;
    i_message_sink*             m_output_sink;

    frame_info_t                m_audio_track_info;
    frame_info_t                m_video_track_info;

    selection_handler_t         m_selection_handler;

public:
    media_frame_selector(const config_t& config
                         , i_message_sink* output_sink = nullptr);

    void set_sink(i_message_sink* output_sink);
    void reset();

    const frame_info_t& audio_track() const;
    const frame_info_t& video_track() const;

    void set_handler(const selection_handler_t& handler);

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;

private:
    bool send_frame(const i_media_frame& frame);
    bool select_frame(const i_media_frame& frame);
};

}

#endif // MPL_MEDIA_FRAME_SELECTOR_H
