#ifndef MPL_MEDIA_FRAME_BUFFER_H
#define MPL_MEDIA_FRAME_BUFFER_H

#include "core/i_message_sink.h"
#include "i_media_frame.h"
#include <map>

namespace mpl::media
{

class media_frame_buffer : public i_message_sink
{
public:
    struct config_t
    {
        timestamp_t     sync_duration;

        config_t(timestamp_t sync_duration = timestamp_null);

        bool is_transit() const;
    };
private:

    using frame_map_t = std::multimap<timestamp_t, i_media_frame::u_ptr_t>;

    config_t                m_config;
    i_message_sink*         m_output_sink;

    frame_map_t             m_frames;

public:
    media_frame_buffer(const config_t& config
                 , i_message_sink* output_sink = {});

    void set_sink(i_message_sink* output_sink);
    std::size_t pending_frames() const;
    timestamp_t delay() const;
    std::size_t process();
    void reset();

    // i_message_sink interface
public:
    bool send_message(const i_message &message) override;

private:
    bool push_frame(timestamp_t timestamp
                    , i_media_frame::u_ptr_t&& frame);
    bool send_frame(const i_media_frame& frame);
    i_media_frame::u_ptr_t fetch_frame();

};

}

#endif // MPL_MEDIA_FRAME_BUFFER_H
