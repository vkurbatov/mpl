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
        p.add_value(value);
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
    }
    return p.close_object();
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
                default:;
            }
        }
    }
    return d.close_object();
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

}
