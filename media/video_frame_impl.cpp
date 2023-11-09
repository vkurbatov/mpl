#include "video_frame_impl.h"
#include "media_module_types.h"
#include "utils/pointer_utils.h"
#include "utils/time_utils.h"
#include "track_info.h"

namespace mpl::media
{

video_frame_base_impl::video_frame_base_impl(frame_id_t frame_id
                                             , timestamp_t timestamp
                                             , video_frame_type_t frame_type)
    : m_frame_info(frame_id
                   , timestamp)
    , m_frame_type(frame_type)
{
    track_info_t::default_video_track().store(m_options);
}

video_frame_base_impl::video_frame_base_impl(const media_frame_info_t &frame_info
                                             , video_frame_type_t frame_type)
    : m_frame_info(frame_info)
    , m_frame_type(frame_type)
{

}

void video_frame_base_impl::set_frame_info(const media_frame_info_t &frame_info)
{
    m_frame_info = frame_info;
}

void video_frame_base_impl::set_frame_id(frame_id_t frame_id)
{
    m_frame_info.frame_id = frame_id;
}

void video_frame_base_impl::set_timestamp(timestamp_t timestamp)
{
    m_frame_info.timestamp = timestamp;
}

void video_frame_base_impl::set_ntp_timestamp(timestamp_t timestamp)
{
    m_frame_info.ntp_timestamp = timestamp;
}

void video_frame_base_impl::set_buffers(smart_buffer_collection &&buffers)
{
    m_buffers = std::move(buffers);
}

void video_frame_base_impl::set_frame_type(video_frame_type_t frame_type)
{
    m_frame_type = frame_type;
}

const media_frame_info_t &video_frame_base_impl::frame_info() const
{
    return m_frame_info;
}

smart_buffer_collection &video_frame_base_impl::smart_buffers()
{
    return m_buffers;
}

const smart_buffer_collection &video_frame_base_impl::smart_buffers() const
{
    return m_buffers;
}

message_category_t video_frame_base_impl::category() const
{
    return message_category_t::data;
}

module_id_t video_frame_base_impl::module_id() const
{
    return media_module_id;
}

const i_option& video_frame_base_impl::options() const
{
    return m_options;
}

media_type_t video_frame_base_impl::media_type() const
{
    return media_type_t::video;
}

frame_id_t video_frame_base_impl::frame_id() const
{
    return m_frame_info.frame_id;
}

timestamp_t video_frame_base_impl::timestamp() const
{
    return m_frame_info.timestamp;
}

timestamp_t video_frame_base_impl::ntp_timestamp() const
{
    return m_frame_info.ntp_timestamp;
}

const i_buffer_collection &video_frame_base_impl::data() const
{
    return m_buffers;
}

video_frame_type_t video_frame_base_impl::frame_type() const
{
    return m_frame_type;
}

media_data_type_t video_frame_base_impl::data_type() const
{
    return media_data_type_t::frame;
}


video_frame_impl::u_ptr_t video_frame_impl::create(const video_format_impl &video_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp
                                                   , video_frame_type_t frame_type)
{
    return std::make_unique<video_frame_impl>(video_format
                                              , frame_id
                                              , timestamp
                                              , frame_type);
}

video_frame_impl::u_ptr_t video_frame_impl::create(video_format_impl &&video_format
                                                   , frame_id_t frame_id
                                                   , timestamp_t timestamp
                                                   , video_frame_type_t frame_type)
{
    return std::make_unique<video_frame_impl>(std::move(video_format)
                                              , frame_id
                                              , timestamp
                                              , frame_type);
}

video_frame_impl::u_ptr_t video_frame_impl::create(const i_video_frame &other)
{
    return std::make_unique<video_frame_impl>(other);
}

video_frame_impl::video_frame_impl(const video_format_impl &video_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp
                                   , video_frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(video_format)
{

}

video_frame_impl::video_frame_impl(video_format_impl &&video_format
                                   , frame_id_t frame_id
                                   , timestamp_t timestamp
                                   , video_frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(std::move(video_format))
{

}

video_frame_impl::video_frame_impl(const i_video_frame &other)
    : video_frame_impl(other.format()
                       , other.frame_id()
                       , other.timestamp()
                       , other.frame_type())
{
    set_ntp_timestamp(other.ntp_timestamp());
    m_buffers.assign(other.data());
}

void video_frame_impl::set_format(const video_format_impl &video_format)
{
    m_video_format = video_format;
}

void video_frame_impl::set_format(video_format_impl &&video_format)
{
    m_video_format = std::move(video_format);
}

void video_frame_impl::set_format(const i_video_format& video_format)
{
    m_video_format.assign(video_format);
}

video_format_impl &video_frame_impl::video_format()
{
    return m_video_format;
}

void video_frame_impl::assign(const i_video_frame &other)
{
    m_video_format.assign(other.format());
    m_frame_info.frame_id = other.frame_id();
    m_frame_info.timestamp = other.timestamp();
    m_frame_info.ntp_timestamp = other.ntp_timestamp();
    m_frame_type = other.frame_type();
    m_buffers.assign(other.data());
}

i_message::u_ptr_t video_frame_impl::clone() const
{
    if (auto clone_frame = create(m_video_format
                                  , m_frame_info.frame_id
                                  , m_frame_info.timestamp
                                  , m_frame_type))
    {
        clone_frame->set_ntp_timestamp(m_frame_info.ntp_timestamp);
        clone_frame->m_options = m_options;
        clone_frame->m_buffers = m_buffers.fork();
        return clone_frame;
    }

    return nullptr;
}

const i_video_format &video_frame_impl::format() const
{
    return m_video_format;
}

video_frame_ptr_impl::u_ptr_t video_frame_ptr_impl::create(const i_video_format::s_ptr_t &video_format
                                                           , frame_id_t frame_id
                                                           , timestamp_t timestamp
                                                           , video_frame_type_t frame_type)
{
    return std::make_unique<video_frame_ptr_impl>(video_format
                                                  , frame_id
                                                  , timestamp
                                                  , frame_type);
}

video_frame_ptr_impl::u_ptr_t video_frame_ptr_impl::create(const i_video_frame &other)
{
    return std::make_unique<video_frame_ptr_impl>(other);
}

video_frame_ptr_impl::video_frame_ptr_impl(const i_video_format::s_ptr_t &video_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp
                                           , video_frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format_ptr(video_format)
{

}

video_frame_ptr_impl::video_frame_ptr_impl(const i_video_frame &other)
    : video_frame_ptr_impl(utils::static_pointer_cast<i_video_format>(other.format().clone())
                           , other.frame_id()
                           , other.timestamp()
                           , other.frame_type())
{
    m_buffers.assign(other.data());
}

void video_frame_ptr_impl::set_format(const i_video_format::s_ptr_t &video_format)
{
    m_video_format_ptr = video_format;
}

void video_frame_ptr_impl::assign(const i_video_frame &other)
{
    m_video_format_ptr = utils::static_pointer_cast<i_video_format>(other.format().clone());
    m_frame_info.frame_id = other.frame_id();
    m_frame_info.timestamp = other.timestamp();
    m_frame_info.ntp_timestamp = other.ntp_timestamp();
    m_frame_type = other.frame_type();
    m_buffers.assign(other.data());
}

i_message::u_ptr_t video_frame_ptr_impl::clone() const
{
    if (m_video_format_ptr)
    {
        if (auto clone_format = utils::static_pointer_cast<i_video_format>(m_video_format_ptr->clone()))
        {
            if (auto clone_frame = create(std::move(clone_format)
                                          , m_frame_info.frame_id
                                          , m_frame_info.timestamp
                                          , m_frame_type))
            {
                clone_frame->set_ntp_timestamp(m_frame_info.ntp_timestamp);
                clone_frame->m_options = m_options;
                clone_frame->m_buffers = m_buffers.fork();
                return clone_frame;
            }
        }
    }

    return nullptr;
}

const i_video_format &video_frame_ptr_impl::format() const
{
    return *m_video_format_ptr;
}

video_frame_ref_impl::video_frame_ref_impl(const i_video_format &video_format
                                           , frame_id_t frame_id
                                           , timestamp_t timestamp
                                           , video_frame_type_t frame_type)
    : video_frame_base_impl(frame_id
                            , timestamp
                            , frame_type)
    , m_video_format(video_format)
{

}

i_message::u_ptr_t video_frame_ref_impl::clone() const
{
    if (auto clone_format = utils::static_pointer_cast<i_video_format>(format().clone()))
    {
        if (auto clone_frame = video_frame_ptr_impl::create(std::move(clone_format)
                                                           , m_frame_info.frame_id
                                                           , m_frame_info.timestamp
                                                           , m_frame_type))
        {
            clone_frame->set_ntp_timestamp(m_frame_info.ntp_timestamp);
            clone_frame->set_options(m_options);
            clone_frame->set_buffers(m_buffers.fork());
            return clone_frame;
        }
    }

    return nullptr;
}

const i_video_format &video_frame_ref_impl::format() const
{
    return m_video_format;
}


}
