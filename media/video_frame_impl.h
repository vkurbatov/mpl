#ifndef MPL_VIDEO_FRAME_IMPL_H
#define MPL_VIDEO_FRAME_IMPL_H

#include "i_video_frame.h"
#include "video_format_impl.h"
#include "core/smart_buffer_collection.h"
#include <unordered_map>

namespace mpl::media
{

class video_frame_base_impl : public i_video_frame
{
protected:
    smart_buffer_collection         m_buffers;
    frame_id_t                      m_frame_id;
    timestamp_t                     m_timestamp;
    i_video_frame::frame_type_t     m_frame_type;

public:
    using u_ptr_t = std::unique_ptr<video_frame_base_impl>;
    using s_ptr_t = std::shared_ptr<video_frame_base_impl>;

    video_frame_base_impl(frame_id_t frame_id = frame_id_undefined
                         , timestamp_t timestamp = timestamp_infinite
                         , i_video_frame::frame_type_t frame_type
                          = i_video_frame::frame_type_t::undefined);

    void set_frame_id(frame_id_t frame_id);
    void set_timestamp(timestamp_t timestamp);
    void set_buffers(smart_buffer_collection&& buffers);
    void set_frame_type(i_video_frame::frame_type_t frame_type);

    smart_buffer_collection& smart_buffers();
    const smart_buffer_collection& smart_buffers() const;


    // i_media_frame interface
public:
    media_type_t media_type() const override;
    frame_id_t frame_id() const override;
    timestamp_t timestamp() const override;
    const i_buffer_collection& buffers() const override;

    // i_video_frame interface
public:
    frame_type_t frame_type() const override;
};

class video_frame_impl : public video_frame_base_impl
{
    video_format_impl   m_video_format;
public:
    using u_ptr_t = std::unique_ptr<video_frame_impl>;
    using s_ptr_t = std::shared_ptr<video_frame_impl>;

    static u_ptr_t create(const video_format_impl& video_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite
                          , i_video_frame::frame_type_t frame_type
                          = i_video_frame::frame_type_t::undefined);

    static u_ptr_t create(video_format_impl&& video_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite
                          , i_video_frame::frame_type_t frame_type
                          = i_video_frame::frame_type_t::undefined);

    static u_ptr_t create(const i_video_frame& other);

    video_frame_impl(const video_format_impl& video_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite
                     , i_video_frame::frame_type_t frame_type
                     = i_video_frame::frame_type_t::undefined);

    video_frame_impl(video_format_impl&& video_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite
                     , i_video_frame::frame_type_t frame_type
                     = i_video_frame::frame_type_t::undefined);

    video_frame_impl(const i_video_frame& other);

    void set_format(const video_format_impl& video_format);
    void set_format(video_format_impl&& video_format);
    void set_format(const i_video_format& video_format);

    video_format_impl& video_format();

    void assign(const i_video_frame& other);

    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;

    // i_video_frame interface
public:
    const i_video_format &format() const override;
};

class video_frame_ptr_impl : public video_frame_base_impl
{
    i_video_format::s_ptr_t   m_video_format_ptr;
public:
    using u_ptr_t = std::unique_ptr<video_frame_ptr_impl>;
    using s_ptr_t = std::shared_ptr<video_frame_ptr_impl>;

    static u_ptr_t create(const i_video_format::s_ptr_t& video_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite
                          , i_video_frame::frame_type_t frame_type
                          = i_video_frame::frame_type_t::undefined);

    static u_ptr_t create(const i_video_frame& other);


    video_frame_ptr_impl(const i_video_format::s_ptr_t& video_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite
                     , i_video_frame::frame_type_t frame_type
                         = i_video_frame::frame_type_t::undefined);

    video_frame_ptr_impl(const i_video_frame& other);


    void set_format(const i_video_format::s_ptr_t& video_format);
    void assign(const i_video_frame& other);


    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;

    // i_video_frame interface
public:
    const i_video_format &format() const override;
};

class video_frame_ref_impl : public video_frame_base_impl
{
    const i_video_format&           m_video_format;
public:
    using u_ptr_t = std::unique_ptr<video_frame_ptr_impl>;
    using s_ptr_t = std::shared_ptr<video_frame_ptr_impl>;

    video_frame_ref_impl(const i_video_format& video_format
                        , frame_id_t frame_id = frame_id_undefined
                        , timestamp_t timestamp = timestamp_infinite
                        , i_video_frame::frame_type_t frame_type
                            = i_video_frame::frame_type_t::undefined);


    // i_media_frame interface
public:
    i_media_frame::u_ptr_t clone() const override;

    // i_video_frame interface
public:
    const i_video_format &format() const override;
};


}

#endif // MPL_VIDEO_FRAME_IMPL_H
