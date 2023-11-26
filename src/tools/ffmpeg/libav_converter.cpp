#include "libav_converter.h"

#include <cstring>

extern "C"
{
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/pixdesc.h>
}

#include <iostream>

namespace pt::ffmpeg
{

namespace utils
{

bool fill_frame(AVPicture& picture
               , void* const slice_list[]
               , pixel_format_t pixel_format
               , const frame_size_t& frame_size)
{
    if (av_image_fill_linesizes(picture.linesize
                                , static_cast<AVPixelFormat>(pixel_format)
                                , frame_size.width) >= 0)
    {
        std::memcpy(picture.data
                    , slice_list
                    , sizeof(void*) * video_info_t::planes(pixel_format));
        return true;
    }

    return false;
}

bool crop_frame(AVPicture& picture
               , pixel_format_t pixel_format
               , const frame_point_t& frame_point)
{
    return av_picture_crop((AVPicture*)&picture
                          , (AVPicture*)&picture
                          , static_cast<AVPixelFormat>(pixel_format)
                          , frame_point.y
                          , frame_point.x) >= 0;
}

std::size_t prepare_frame(AVPicture& picture
                         , const frame_rect_t& frame_rect
                         , pixel_format_t pixel_format
                         , void * const slice_list[]
                         , std::int32_t align = default_frame_align)
{

    if (fill_frame(picture
                   , slice_list
                   , pixel_format
                   , frame_rect.size)
            && crop_frame(picture
                          , pixel_format
                          , frame_rect.offset))
    {
        return video_info_t::frame_size(pixel_format
                                        , frame_rect.size
                                        , align);
    }

    return 0;
}

}

struct libav_converter_context_t
{ 
    struct SwsContext*  m_sws_context;
    frame_size_t        m_input_frame_size;
    pixel_format_t      m_input_pixel_format;
    frame_size_t        m_output_frame_size;
    pixel_format_t      m_output_pixel_format;
    scaling_method_t    m_scaling_method;
    std::int32_t        m_linesize_align;

    libav_converter_context_t(scaling_method_t scaling_method
                              , std::int32_t linesize_align)
        : m_sws_context(nullptr)
        , m_scaling_method(scaling_method)
        , m_linesize_align(linesize_align)
    {

    }

    ~libav_converter_context_t()
    {
        reset();
    }   

    bool check_or_create_context(const frame_size_t& input_frame_size
                                 , pixel_format_t input_pixel_format
                                 , const frame_size_t& output_frame_size
                                 , pixel_format_t output_pixel_format)
    {
        if (m_sws_context == nullptr
                || m_input_pixel_format != input_pixel_format
                || m_input_frame_size != input_frame_size
                || m_output_pixel_format != output_pixel_format
                || m_output_frame_size != output_frame_size)
        {
            m_sws_context = sws_getCachedContext(m_sws_context
                                                 , input_frame_size.width
                                                 , input_frame_size.height
                                                 , static_cast<AVPixelFormat>(input_pixel_format)
                                                 , output_frame_size.width
                                                 , output_frame_size.height
                                                 , static_cast<AVPixelFormat>(output_pixel_format)
                                                 , static_cast<std::uint32_t>(m_scaling_method)
                                                 , nullptr
                                                 , nullptr
                                                 , nullptr);

            if (m_sws_context != nullptr)
            {
                m_input_pixel_format = input_pixel_format;
                m_input_frame_size = input_frame_size;
                m_output_pixel_format = output_pixel_format;
                m_output_frame_size = output_frame_size;
            }
        }

        return m_sws_context != nullptr;
    }   

    std::size_t scale(const fragment_info_t& input_fragment_info
                      , void* const input_slices[]
                      , const fragment_info_t& output_fragment_info
                      , void* output_slices[])
    {
        std::size_t result = 0;

        if (check_or_create_context(input_fragment_info.frame_rect.size
                                                 , input_fragment_info.pixel_format
                                                 , output_fragment_info.frame_rect.size
                                                 , output_fragment_info.pixel_format))
        {



            frame_size_t input_frame_size = input_fragment_info.frame_size;
            frame_size_t output_frame_size = output_fragment_info.frame_size;


            /*input_frame_size.width -= input_frame_size.width % default_frame_align;
            output_frame_size.width -= output_frame_size.width % default_frame_align;*/

            /*
            if (input_frame_size.width % default_frame_align != 0)
            {
                input_frame_size.width += (default_frame_align - input_frame_size.width % default_frame_align);
            }

            if (output_frame_size.width % default_frame_align != 0)
            {
                output_frame_size.width += (default_frame_align - output_frame_size.width % default_frame_align);
            }*/


            AVPicture src_frame = {};
            AVPicture dst_frame = {};

            auto sz_input = utils::prepare_frame(src_frame
                                                 , { input_fragment_info.frame_rect.offset, input_frame_size }
                                                 , input_fragment_info.pixel_format
                                                 , input_slices
                                                 , m_linesize_align);

            auto sz_output = utils::prepare_frame(dst_frame
                                                 , { output_fragment_info.frame_rect.offset, output_frame_size }
                                                 , output_fragment_info.pixel_format
                                                 , output_slices
                                                 , m_linesize_align);


            if (sz_input != 0
                    && sz_output != 0)
            {

                auto h_corr = sz_input == sz_output
                        && input_fragment_info.frame_rect.size == output_fragment_info.frame_rect.size
                            ? 2
                            : 0;

                auto sws_result = sws_scale(m_sws_context
                                            , src_frame.data
                                            , src_frame.linesize
                                            , h_corr
                                            , input_fragment_info.frame_rect.size.height - h_corr
                                            , dst_frame.data
                                            , dst_frame.linesize);

                if (sws_result > 0)
                {
                    // std::cout << "src_stride = " << src_stride[0] << ", dst_stride = " << dst_stride[0] << std::endl;
                    result = sz_output;
                }
            }

        }

        return result;
    }

    std::size_t convert_frames(const fragment_info_t& input_fragment_info
                               , const void* input_frame
                               , const fragment_info_t& output_fragment_info
                               , void* output_frame)
    {

        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info)
        {
            result = input_fragment_info.get_frame_size();
            if (output_frame != input_frame)
            {
                std::memcpy(output_frame
                            , input_frame
                            , result);
            }
        }
        else
        {


            void *input_slices[max_planes] = {};
            void *output_slices[max_planes] = {};

            video_info_t::split_slices(input_fragment_info.pixel_format
                                       , input_fragment_info.frame_size
                                       , input_slices
                                       , input_frame);


            video_info_t::split_slices(output_fragment_info.pixel_format
                                       , output_fragment_info.frame_size
                                       , output_slices
                                       , output_frame);


            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices);


        }

        return result;
    }

    std::size_t convert_slices(const fragment_info_t& input_fragment_info
                               , void* const input_slices[]
                               , const fragment_info_t& output_fragment_info
                               , void* output_slices[])
    {
        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info)
        {
            result = input_fragment_info.get_frame_size();

            if (input_slices != output_slices)
            {
                auto i = 0;
                for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                                , input_fragment_info.frame_size))
                {
                    if (input_slices[i] != output_slices[i])
                    {
                        std::memcpy(output_slices[i]
                                    , input_slices[i]
                                    , sz.size());
                    }
                    i++;
                }
            }
        }
        else
        {
            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices);
        }

        return result;
    }

    std::size_t convert_to_slices(const fragment_info_t& input_fragment_info
                                  , const void* input_frame
                                  , const fragment_info_t& output_fragment_info
                                  , void* output_slices[])
    {

        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info)
        {
            result = input_fragment_info.get_frame_size();

            auto i = 0;
            std::size_t offset = 0;
            for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                            , input_fragment_info.frame_size))
            {
                std::memcpy(output_slices[i]
                            , static_cast<const std::uint8_t*>(input_frame) + offset
                            , sz.size());

                offset += sz.size();
                i++;
            }
        }
        else
        {

            void *input_slices[max_planes] = {};

            video_info_t::split_slices(input_fragment_info.pixel_format
                                       , input_fragment_info.frame_size
                                       , input_slices
                                       , input_frame);

            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices);


        }

        return result;
    }

    std::size_t convert_to_frame(const fragment_info_t& input_fragment_info
                                  , void* const input_slices[]
                                  , const fragment_info_t& output_fragment_info
                                  , void* output_frame)
    {
        std::size_t result = 0;

        if (input_fragment_info.is_full()
                && input_fragment_info == output_fragment_info)
        {
            result = input_fragment_info.get_frame_size();

            auto i = 0;
            std::size_t offset = 0;
            for (const auto& sz : video_info_t::plane_sizes(input_fragment_info.pixel_format
                                                            , input_fragment_info.frame_size))
            {
                std::memcpy(static_cast<std::uint8_t*>(output_frame) + offset
                            , input_slices[i]
                            , sz.size());

                offset += sz.size();
                i++;
            }
        }
        else
        {
            void *output_slices[max_planes] = {};

            video_info_t::split_slices(output_fragment_info.pixel_format
                                       , output_fragment_info.frame_size
                                       , output_slices
                                       , output_frame);

            result = scale(input_fragment_info
                           , input_slices
                           , output_fragment_info
                           , output_slices);
        }

        return result;
    }


    void reset()
    {
        if (m_sws_context != nullptr)
        {
            sws_freeContext(m_sws_context);
            m_sws_context = nullptr;
        }
    }

    void reset(scaling_method_t scaling_method)
    {
        reset();

        m_scaling_method = scaling_method;
    }

};
// -----------------------------------------------------------------------------
void libav_converter_context_deleter_t::operator()(libav_converter_context_t *libav_converter_context_ptr)
{
    delete libav_converter_context_ptr;
}
// -----------------------------------------------------------------------------

#define CHECK_FORMATS if (!input_fragment_info.is_convertable() || !output_fragment_info.is_convertable()) return 0

libav_converter::u_ptr_t libav_converter::create(scaling_method_t scaling_method
                                                 , int32_t linesize_align)
{
    return std::make_unique<libav_converter>(scaling_method
                                             , linesize_align);
}

libav_converter::libav_converter(scaling_method_t scaling_method
                                 , std::int32_t linesize_align)
    : m_converter_context(new libav_converter_context_t(scaling_method
                                                        , linesize_align))
{

}

std::size_t libav_converter::convert_frames(const fragment_info_t& input_fragment_info
                                           , const void* input_frame
                                           , const fragment_info_t& output_fragment_info
                                           , void* output_frame)
{
    return m_converter_context->convert_frames(input_fragment_info
                                               , input_frame
                                               , output_fragment_info
                                               , output_frame);
}

std::size_t libav_converter::convert_slices(const fragment_info_t &input_fragment_info
                                            , void * const input_slices[]
                                            , const fragment_info_t &output_fragment_info
                                            , void *output_slices[])
{
    CHECK_FORMATS;
    return m_converter_context->convert_slices(input_fragment_info
                                               , input_slices
                                               , output_fragment_info
                                               , output_slices);
}

std::size_t libav_converter::convert_to_slices(const fragment_info_t &input_fragment_info
                                               , const void *input_frame
                                               , const fragment_info_t &output_fragment_info
                                               , void *output_slices[])
{
    CHECK_FORMATS;
    return m_converter_context->convert_to_slices(input_fragment_info
                                                 , input_frame
                                                 , output_fragment_info
                                                 , output_slices);
}

std::size_t libav_converter::convert_to_frame(const fragment_info_t &input_fragment_info
                                              , void * const input_slices[]
                                              , const fragment_info_t &output_fragment_info
                                              , void *output_frame)
{
    CHECK_FORMATS;
    return m_converter_context->convert_to_frame(input_fragment_info
                                                 , input_slices
                                                 , output_fragment_info
                                                 , output_frame);
}

void libav_converter::reset(scaling_method_t scaling_method)
{
    m_converter_context->reset(scaling_method);
}

void libav_converter::reset()
{
    m_converter_context->reset();
}

scaling_method_t libav_converter::scaling_method() const
{
    return m_converter_context->m_scaling_method;
}


}
