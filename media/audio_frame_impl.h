#ifndef MPL_AUDIO_FRAME_IMPL_H
#define MPL_AUDIO_FRAME_IMPL_H

#include "i_audio_frame.h"
#include "audio_format_impl.h"
#include "media_frame_info.h"
#include "utils/smart_buffer_collection.h"
#include "frame_option_container.h"
#include <unordered_map>

namespace mpl::media
{

class audio_frame_base_impl : public i_audio_frame
        , public frame_option_container
{
protected:
    smart_buffer_collection     m_buffers;
    media_frame_info_t          m_frame_info;

public:
    using u_ptr_t = std::unique_ptr<audio_frame_base_impl>;
    using s_ptr_t = std::shared_ptr<audio_frame_base_impl>;


    audio_frame_base_impl(frame_id_t frame_id = frame_id_undefined
                         , timestamp_t timestamp = timestamp_infinite);

    audio_frame_base_impl(const media_frame_info_t& frame_info);

    void set_frame_info(const media_frame_info_t& frame_info);
    void set_frame_id(frame_id_t frame_id);
    void set_timestamp(timestamp_t timestamp);
    void set_ntp_timestamp(timestamp_t ntp_timestamp);
    void set_buffers(smart_buffer_collection&& buffers);
    smart_buffer_collection& smart_buffers();
    const smart_buffer_collection& smart_buffers() const;

    const media_frame_info_t& frame_info() const;

    // i_message interface
public:
    message_category_t category() const override;

    // i_message_data interface
public:
    message_subclass_t subclass() const override;
    const i_option& options() const override;

    // i_media_frame interface
public:
    media_type_t media_type() const override;
    frame_id_t frame_id() const override;
    timestamp_t timestamp() const override;
    timestamp_t ntp_timestamp() const override;
    const i_buffer_collection& data() const override;

    // i_message_media_data interface
public:
    media_data_type_t data_type() const override;
};

class audio_frame_impl final : public audio_frame_base_impl
{
    audio_format_impl   m_audio_format;
public:
    using u_ptr_t = std::unique_ptr<audio_frame_impl>;
    using s_ptr_t = std::shared_ptr<audio_frame_impl>;

    static u_ptr_t create(const audio_format_impl& audio_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite);

    static u_ptr_t create(audio_format_impl&& audio_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite);

    static u_ptr_t create(const i_audio_frame& other);

    audio_frame_impl(const audio_format_impl& audio_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite);

    audio_frame_impl(audio_format_impl&& audio_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite);

    audio_frame_impl(const i_audio_frame& other);

    audio_format_impl& audio_format();

    void set_format(const audio_format_impl& audio_format);
    void set_format(audio_format_impl&& audio_format);
    void set_format(const i_audio_format& audio_format);
    void assign(const i_audio_frame& other);

    // i_media_frame interface
public:
    i_message::u_ptr_t clone() const override;

    // i_audio_frame interface
public:
    const i_audio_format &format() const override;
};

class audio_frame_ptr_impl final : public audio_frame_base_impl
{
    i_audio_format::s_ptr_t   m_audio_format_ptr;
public:
    using u_ptr_t = std::unique_ptr<audio_frame_ptr_impl>;
    using s_ptr_t = std::shared_ptr<audio_frame_ptr_impl>;

    static u_ptr_t create(const i_audio_format::s_ptr_t& audio_format
                          , frame_id_t frame_id = frame_id_undefined
                          , timestamp_t timestamp = timestamp_infinite);

    static u_ptr_t create(const i_audio_frame& other);

    audio_frame_ptr_impl(const i_audio_format::s_ptr_t& audio_format
                     , frame_id_t frame_id = frame_id_undefined
                     , timestamp_t timestamp = timestamp_infinite);

    audio_frame_ptr_impl(const i_audio_frame& other);


    void set_format(const i_audio_format::s_ptr_t& audio_format);
    void set_format(const i_audio_format& audio_format);
    void assign(const i_audio_frame& other);


    // i_media_frame interface
public:
    i_message::u_ptr_t clone() const override;

    // i_audio_frame interface
public:
    const i_audio_format &format() const override;
};

class audio_frame_ref_impl final : public audio_frame_base_impl
{
    const i_audio_format&           m_audio_format;
public:
    using u_ptr_t = std::unique_ptr<audio_frame_ptr_impl>;
    using s_ptr_t = std::shared_ptr<audio_frame_ptr_impl>;

    audio_frame_ref_impl(const i_audio_format& audio_format
                        , frame_id_t frame_id = frame_id_undefined
                        , timestamp_t timestamp = timestamp_infinite);


    // i_media_frame interface
public:
    i_message::u_ptr_t clone() const override;

    // i_audio_frame interface
public:
    const i_audio_format &format() const override;
};

}

#endif // MPL_AUDIO_FRAME_IMPL_H
