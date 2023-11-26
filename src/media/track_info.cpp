#include "track_info.h"
#include "utils/option_helper.h"
#include "media_option_types.h"

namespace mpl::media
{

namespace detail
{

template<typename T>
inline bool has_compatible(const T& lhs, const T& rhs, const T& undefined)
{
    return lhs == rhs
            || rhs == undefined;
}

}

const track_info_t &track_info_t::default_audio_track()
{
    static const track_info_t single_audio_track_info(default_stream_id
                                                        , default_audio_track_id
                                                        , default_layer_id);

    return single_audio_track_info;
}

const track_info_t &track_info_t::default_video_track()
{
    static const track_info_t single_video_track_info(default_stream_id
                                                       , default_video_track_id
                                                       , default_layer_id);

    return single_video_track_info;
}

const track_info_t &track_info_t::default_data_track()
{
    static const track_info_t single_data_track_info(default_stream_id
                                                       , default_data_track_id
                                                       , default_layer_id);

    return single_data_track_info;
}

track_info_t::track_info_t(stream_id_t stream_id
                           , track_id_t track_id
                           , layer_id_t layer_id)
    : stream_id(stream_id)
    , track_id(track_id)
    , layer_id(layer_id)
{

}

track_info_t::track_info_t(const i_option &options)
    : track_info_t()
{
    load(options);
}

bool track_info_t::load(const i_option &options)
{
    option_reader reader(options);
    return reader.get(opt_frm_stream_id, stream_id)
            | reader.get(opt_frm_track_id, track_id)
            | reader.get(opt_frm_layer_id, layer_id);
}

bool track_info_t::store(i_option &options) const
{
    option_writer writer(options);
    return writer.set(opt_frm_stream_id, stream_id, stream_id_undefined)
            && writer.set(opt_frm_track_id, track_id, track_id_undefined)
            && writer.set(opt_frm_layer_id, layer_id, layer_id_undefined);
}

bool track_info_t::operator ==(const track_info_t &other) const
{
    return stream_id == other.stream_id
            && track_id == other.track_id
            && layer_id == other.layer_id;
}

bool track_info_t::operator !=(const track_info_t &other) const
{
    return ! operator == (other);
}

std::size_t track_info_t::hash() const
{
    return std::hash<stream_id_t>()(stream_id)
            ^ std::hash<track_id_t>()(track_id)
            ^ std::hash<layer_id_t>()(layer_id);
}

bool track_info_t::is_define() const
{
    return stream_id >= stream_id_undefined
            || track_id >= track_id_undefined;
}

bool track_info_t::is_compatible(const track_info_t &other) const
{
    return detail::has_compatible(stream_id, other.stream_id, stream_id_undefined)
            && detail::has_compatible(track_id, other.track_id, track_id_undefined)
            && detail::has_compatible(layer_id, other.layer_id, layer_id_undefined);
}

}

std::size_t std::hash<mpl::media::track_info_t>::operator()(const mpl::media::track_info_t &s) const noexcept
{
    return s.hash();
}
