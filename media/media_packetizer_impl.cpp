#include "core/packetizer.h"
#include "core/depacketizer.h"
#include "core/option_helper.h"

#include "media_utils.h"
#include "audio_format_impl.h"
#include "video_format_impl.h"
#include "audio_frame_impl.h"
#include "video_frame_impl.h"

#include "media_option_types.h"

namespace mpl
{

using namespace mpl::media;

namespace detail
{

template<option_id_t ID, typename T>
bool packetize_option_value(packetizer& p
                            , const option_reader& reader)
{
    if (auto value = reader.get<T>(ID))
    {
        p.add_value(ID);
        p.add_value(*value);
        return true;
    }

    return false;
}

bool packetize_media_options(packetizer& p
                             , const i_option& options)
{
    if (p.open_object())
    {
        option_reader reader(options);
        packetize_option_value<opt_fmt_stream_id, std::int32_t>(p , reader);
        packetize_option_value<opt_fmt_device_id, std::int32_t>(p , reader);
        packetize_option_value<opt_codec_extra_data, octet_string_t>(p , reader);
        packetize_option_value<opt_codec_params, std::string>(p , reader);

        return p.close_object();
    }

    return false;
}

template<option_id_t ID, typename T>
bool depacketize_option_value(depacketizer& d
                              , option_writer& writer)
{
    T value;
    if (d.fetch_value(value))
    {
        return writer.set<T>(ID, value);
    }

    return false;
}

bool depacketize_media_options(depacketizer& d
                             , i_option& options)
{
    if (d.open_object())
    {
        option_writer writer(options);
        option_id_t option_id = 0;
        while (d.fetch_value(option_id))
        {
            switch(option_id)
            {
                case opt_fmt_stream_id:
                    depacketize_option_value<opt_fmt_stream_id, std::int32_t>(d, writer);
                break;
                case opt_fmt_device_id:
                    depacketize_option_value<opt_fmt_device_id, std::int32_t>(d, writer);
                break;
                case opt_codec_extra_data:
                    depacketize_option_value<opt_codec_extra_data, octet_string_t>(d, writer);
                break;
                case opt_codec_params:
                    depacketize_option_value<opt_codec_params, std::string>(d, writer);
                break;
                default:;
            }
        }

        return d.close_object();
    }

    return false;
}

}

// audio format

template<>
bool packetizer::add_value(const i_audio_format& audio_format)
{
    if (open_object())
    {
        if (add_enum(audio_format.media_type())
                && add_enum(audio_format.format_id())
                && add_value(audio_format.sample_rate())
                && add_value(audio_format.channels()))
        {
            detail::packetize_media_options(*this
                                            , audio_format.options());
            return close_object();
        }
    }

    return false;
}


template<>
bool packetizer::add_value(const audio_format_impl& audio_format)
{
    return add_value<i_audio_format>(audio_format);
}

template<>
bool depacketizer::fetch_value(audio_format_impl& audio_format)
{

    if (open_object())
    {
        media_type_t media_type = media_type_t::undefined;
        audio_format_id_t audio_format_id = audio_format_id_t::undefined;
        std::int32_t sample_rate = 0;
        std::int32_t channels = 0;
        if (fetch_enum(media_type)
                && media_type == audio_format.media_type()
                && fetch_enum(audio_format_id)
                && fetch_enum(sample_rate)
                && fetch_enum(channels))
        {
            audio_format.set_format_id(audio_format_id);
            audio_format.set_sample_rate(sample_rate);
            audio_format.set_channels(channels);

            detail::depacketize_media_options(*this
                                             , audio_format.options());

            return close_object();
        }
    }
    return false;
}

// video_format

template<>
bool packetizer::add_value(const i_video_format& video_format)
{
    if (open_object())
    {
        if (add_enum(video_format.media_type())
                && add_enum(video_format.format_id())
                && add_value(video_format.width())
                && add_value(video_format.height())
                && add_value(video_format.frame_rate()))
        {
            detail::packetize_media_options(*this
                                            , video_format.options());
            return close_object();
        }
    }

    return false;
}

template<>
bool packetizer::add_value(const video_format_impl& video_format)
{
    return add_value<i_video_format>(video_format);
}

template<>
bool depacketizer::fetch_value(video_format_impl& video_format)
{
    if (open_object())
    {
        media_type_t media_type = media_type_t::undefined;
        video_format_id_t video_format_id = video_format_id_t::undefined;
        std::int32_t width = 0;
        std::int32_t height = 0;
        double frame_rate = 0.0f;
        if (fetch_enum(media_type)
                && media_type == video_format.media_type()
                && fetch_enum(video_format_id)
                && fetch_value(width)
                && fetch_value(height)
                && fetch_value(frame_rate))
        {
            video_format.set_format_id(video_format_id);
            video_format.set_width(width);
            video_format.set_height(height);
            video_format.set_frame_rate(frame_rate);

            detail::depacketize_media_options(*this
                                             , video_format.options());

            return close_object();
        }
    }
    return false;
}

// media frame
template<>
bool packetizer::add_value(const i_media_format& media_format)
{
    switch(media_format.media_type())
    {
        case media_type_t::audio:
            return add_value<i_audio_format>(static_cast<const i_audio_format&>(media_format));
        break;
        case media_type_t::video:
            return add_value<i_video_format>(static_cast<const i_video_format&>(media_format));
        break;
        default:;
    }

    return false;
}

// buffer collection

template<>
bool packetizer::add_value(const i_buffer_collection& buffers)
{
    if (open_object())
    {
        for (const auto& i : buffers.index_list())
        {
            if (auto b = buffers.get_buffer(i))
            {
                add_value(i);
                add(field_type_t::numeric
                    , b->data()
                    , b->size());
            }
        }
        return close_object();
    }

    return false;
}

template<>
bool depacketizer::fetch_value(smart_buffer_collection& buffer_collection)
{
    if (open_object())
    {
        std::int32_t idx = 0;
        while (fetch_value(idx))
        {
            data_field_t data_field;
            if (fetch(data_field))
            {
                buffer_collection.set_buffer(idx, smart_buffer(data_field.data
                                                               , data_field.size
                                                               , 1));
            }
        }
        return close_object();
    }
    return true;
}

// audio frame

template<>
bool packetizer::add_value(const i_audio_frame& audio_frame)
{
    if (open_object())
    {
        if (add_value(audio_frame.format()))
        {
            add_value(audio_frame.frame_id());
            add_value(audio_frame.timestamp());
            add_value(audio_frame.buffers());
            return close_object();
        }
    }

    return false;
}

template<>
bool depacketizer::fetch_value(audio_frame_impl& audio_frame)
{
    if (open_object())
    {
        frame_id_t frame_id = 0;
        timestamp_t timestamp = 0;

        if (fetch_value(audio_frame.audio_format())
                && fetch_value(frame_id)
                && fetch_value(timestamp)
                && fetch_value(audio_frame.smart_buffers()))
        {
            audio_frame.set_frame_id(frame_id);
            audio_frame.set_timestamp(timestamp);
            return close_object();
        }
    }
    return false;
}

// video frame

template<>
bool packetizer::add_value(const i_video_frame& video_frame)
{
    if (open_object())
    {
        if (add_value(video_frame.format())
                && add_value(video_frame.frame_id())
                && add_value(video_frame.timestamp())
                && add_enum(video_frame.frame_type()))
        {
            add_value(video_frame.buffers());
            return close_object();
        }
    }

    return false;
}

template<>
bool packetizer::add_value(const video_frame_impl& video_frame)
{
    return add_value<i_video_frame>(video_frame);
}

template<>
bool depacketizer::fetch_value(video_frame_impl& video_frame)
{
    if (open_object())
    {
        frame_id_t frame_id = 0;
        timestamp_t timestamp = 0;
        i_video_frame::frame_type_t frame_type = i_video_frame::frame_type_t::undefined;

        if (fetch_value(video_frame.video_format())
                && fetch_value(frame_id)
                && fetch_value(timestamp)
                && fetch_enum(frame_type)
                && fetch_value(video_frame.smart_buffers()))
        {
            video_frame.set_frame_id(frame_id);
            video_frame.set_timestamp(timestamp);
            video_frame.set_frame_type(frame_type);

            return close_object();
        }
    }
    return false;
}

}
