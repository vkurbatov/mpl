#ifndef MPL_I_MEDIA_FRAME_BUILDER_H
#define MPL_I_MEDIA_FRAME_BUILDER_H

#include "core/i_buffer_collection.h"
#include "i_media_frame.h"

namespace mpl::media
{

class i_media_format;

class i_media_frame_builder
{
public:
    using u_ptr_t = std::unique_ptr<i_media_frame_builder>;
    using s_ptr_t = std::shared_ptr<i_media_frame_builder>;

    virtual ~i_media_frame_builder() = default;

    virtual void assign(const i_media_frame& media_frame) = 0;

    virtual void set_frame_id(frame_id_t frame_id) = 0;
    virtual frame_id_t frame_id() const = 0;

    virtual void set_timestamp(timestamp_t timestamp) = 0;
    virtual timestamp_t timestamp() const = 0;

    virtual void set_ntp_timestamp(timestamp_t ntp_timestamp) = 0;
    virtual timestamp_t ntp_timestamp() const = 0;

    virtual void set_frame_buffer(std::int32_t buffer_index
                                    , const void* frame_data
                                    , std::size_t frame_size) = 0;
    virtual const i_buffer* frame_buffer(std::int32_t buffer_index) = 0;

    virtual void set_frame_type(video_frame_type_t frame_type) = 0;
    virtual video_frame_type_t frame_type() const = 0;


    virtual void set_frame_option(const i_option& frame_options) = 0;
    virtual i_option& frame_options() = 0;

    virtual void set_format(const i_media_format& media_format) = 0;
    virtual const i_media_format* media_format() const = 0;

    virtual i_media_frame::u_ptr_t create_frame() = 0;

};

}

#endif // MPL_I_MEDIA_FRAME_BUILDER_H
