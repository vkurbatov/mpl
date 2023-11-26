#ifndef MPL_MEDIA_FRAME_BUILDER_IMPL_H
#define MPL_MEDIA_FRAME_BUILDER_IMPL_H

#include "utils/option_impl.h"
#include "utils/smart_buffer_collection.h"
#include "frame_option_container.h"
#include "media_frame_info.h"
#include "i_media_frame_builder.h"
#include "i_media_format.h"

namespace mpl::media
{

class media_frame_builder_impl : public i_media_frame_builder
        , public frame_option_container
{
    media_frame_info_t          m_frame_info;
    video_frame_type_t          m_frame_type;
    i_media_format::u_ptr_t     m_media_format;
    smart_buffer_collection     m_buffers;

public:

    using u_ptr_t = std::unique_ptr<media_frame_builder_impl>;
    using s_ptr_t = std::shared_ptr<media_frame_builder_impl>;

    static u_ptr_t create();
    static u_ptr_t create(const i_media_frame& media_frame);

    media_frame_builder_impl();
    media_frame_builder_impl(const i_media_frame& media_frame);


    // i_media_frame_builder interface
public:
    void assign(const i_media_frame& media_frame) override;
    void set_frame_id(frame_id_t frame_id) override;
    frame_id_t frame_id() const override;
    void set_timestamp(timestamp_t timestamp) override;
    timestamp_t timestamp() const override;
    void set_ntp_timestamp(timestamp_t ntp_timestamp) override;
    timestamp_t ntp_timestamp() const override;
    void set_frame_type(video_frame_type_t frame_type) override;
    video_frame_type_t frame_type() const override;
    void set_frame_buffer(std::int32_t buffer_index
                          , const void *frame_data
                          , std::size_t frame_size) override;
    const i_buffer *frame_buffer(std::int32_t buffer_index) override;
    void set_frame_option(const i_option &frame_options) override;
    i_option &frame_options() override;
    void set_format(const i_media_format &media_format) override;
    const i_media_format *media_format() const override;
    i_media_frame::u_ptr_t create_frame() override;
};

}

#endif // MPL_MEDIA_FRAME_BUILDER_IMPL_H
