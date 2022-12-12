#ifndef FFMPEG_LIBAV_CONVERTER_H
#define FFMPEG_LIBAV_CONVERTER_H

#include "libav_base.h"

namespace ffmpeg
{

struct libav_converter_context_t;
struct libav_converter_context_deleter_t { void operator()(libav_converter_context_t* libav_converter_context_ptr); };

typedef std::unique_ptr<libav_converter_context_t, libav_converter_context_deleter_t> libav_converter_context_ptr_t;

enum class scaling_method_t
{
    default_method =    0x00,
    fast_bilinear =     0x01,   // best speed
    bilinear =          0x02,
    bicubic =           0x04,
    x =                 0x08,
    point =             0x10,
    area =              0x20,
    bicublin =          0x40,
    gauss =             0x80,
    sinc =              0x100,
    lanczos =           0x200,
    spline =            0x400
};

const scaling_method_t default_scaling_method = scaling_method_t::fast_bilinear;

class libav_converter
{
    libav_converter_context_ptr_t   m_converter_context;
    scaling_method_t                m_scaling_method;

public:
    libav_converter(scaling_method_t scaling_method = default_scaling_method
                    , std::int32_t linesize_align = default_frame_align);

    std::size_t convert_frames(const fragment_info_t& input_fragment_info
                               , const void* input_frame
                               , const fragment_info_t& output_fragment_info
                               , void* output_frame);

    std::size_t convert_slices(const fragment_info_t& input_fragment_info
                               , void* const input_slices[]
                               , const fragment_info_t& output_fragment_info
                               , void* output_slices[]);

    std::size_t convert_to_slices(const fragment_info_t& input_fragment_info
                                , const void * input_frame
                                , const fragment_info_t& output_fragment_info
                                , void* output_slices[]);

    std::size_t convert_to_frame(const fragment_info_t& input_fragment_info
                                 , void* const input_slices[]
                                 , const fragment_info_t& output_fragment_info
                                 , void* output_frame);

    void reset(scaling_method_t scaling_method);
    void reset();

    scaling_method_t scaling_method() const;
};

}

#endif // LIBAV_CONVERTER_H
