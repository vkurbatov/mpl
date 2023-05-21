#ifndef MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H
#define MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H

#include "i_media_converter_factory.h"

namespace mpl::media
{

class libav_video_converter_factory : public i_media_converter_factory
{
public:

    static libav_video_converter_factory& get_instance();

    libav_video_converter_factory();

    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_property& params) override;
};

}

#endif // MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H
