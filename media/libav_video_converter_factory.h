#ifndef MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H
#define MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H

#include "i_media_converter_factory.h"

namespace mpl::media
{

class libav_video_converter_factory : public i_media_converter_factory
{
public:

    libav_video_converter_factory();

    // i_media_converter_factory interface
public:
    i_media_converter::u_ptr_t create_converter(const i_media_format &output_format) override;
};

}

#endif // MPL_LIBAV_VIDEO_CONVERTER_FACTORY_H
