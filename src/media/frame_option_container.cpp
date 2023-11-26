#include "frame_option_container.h"
#include "utils/option_helper.h"
#include "media_option_types.h"

namespace mpl::media
{

frame_option_container::frame_option_container(const i_option &options)
    : utils::option_container(options)
{

}

frame_option_container::frame_option_container(option_impl &&options)
    : utils::option_container(std::move(options))
{

}

stream_id_t frame_option_container::stream_id() const
{
    return option_reader(m_options).get(opt_frm_stream_id, stream_id_undefined);
}

track_id_t frame_option_container::track_id() const
{
    return option_reader(m_options).get(opt_frm_track_id, track_id_undefined);
}

layer_id_t frame_option_container::layer_id() const
{
    return option_reader(m_options).get(opt_frm_layer_id, layer_id_undefined);
}

bool frame_option_container::set_stream_id(stream_id_t stream_id)
{
    return option_writer(m_options).set(opt_frm_stream_id, stream_id);
}

bool frame_option_container::set_track_id(stream_id_t track_id)
{
    return option_writer(m_options).set(opt_frm_track_id, track_id);
}

bool frame_option_container::set_layer_id(stream_id_t layer_id)
{
    return option_writer(m_options).set(opt_frm_layer_id, layer_id);
}


}
