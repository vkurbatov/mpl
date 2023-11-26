#ifndef MPL_MEDIA_TRACK_INFO_H
#define MPL_MEDIA_TRACK_INFO_H

#include "media_types.h"
#include "core/i_option.h"


namespace mpl::media
{

struct track_info_t
{
    stream_id_t     stream_id;
    track_id_t      track_id;
    layer_id_t      layer_id;

    static const track_info_t& default_audio_track();
    static const track_info_t& default_video_track();
    static const track_info_t& default_data_track();

    track_info_t(stream_id_t stream_id = stream_id_undefined
                , track_id_t track_id = track_id_undefined
                , layer_id_t layer_id = layer_id_undefined);

    track_info_t(const i_option& options);

    bool load(const i_option& options);
    bool store(i_option& options) const;


    bool operator == (const track_info_t& other) const;
    bool operator != (const track_info_t& other) const;

    std::size_t hash() const;

    bool is_define() const;
    bool is_compatible(const track_info_t& other) const;
};

}

template<>
struct std::hash<mpl::media::track_info_t>
{
    std::size_t operator()(const mpl::media::track_info_t& s) const noexcept;
};

#endif // MPL_MEDIA_TRACK_INFO_H
