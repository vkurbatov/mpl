#include "video_image_builder.h"
#include "image_frame.h"
#include "draw_options.h"

#include "tools/opencv/draw_processor.h"

#include <cstdint>

namespace mpl::media
{

namespace detail
{

ocv::frame_info_t create_frame_info(const image_frame_t& image_frame)
{
    ocv::frame_info_t frame_info;

    switch(image_frame.format_id)
    {
        case video_format_id_t::bgr24:
            frame_info.format = ocv::frame_format_t::bgr;
        break;
        case video_format_id_t::bgra32:
            frame_info.format = ocv::frame_format_t::bgra;
        break;
        default:;
    }

    frame_info.size = image_frame.size;

    return frame_info;
}

ocv::frame_info_t create_frame_info(const image_frame_t* image_frame)
{
    if (image_frame)
    {
        return create_frame_info(*image_frame);
    }

    return {};
}

}

struct video_image_builder::context_t
{
    using config_t = video_image_builder::config_t;
    using u_ptr_t = video_image_builder::context_ptr_t;

    video_image_builder::config_t   m_config;
    image_frame_t*                  m_output_frame;
    ocv::draw_processor             m_draw_processor;



    static u_ptr_t create(const config_t& config
                          , image_frame_t* output_frame)
    {
        return std::make_unique<context_t>(config
                                           , output_frame);
    }


    context_t(const config_t& config
              , image_frame_t* output_frame)
        : m_config(config)
        , m_output_frame(output_frame)
        , m_draw_processor(detail::create_frame_info(m_output_frame)
                           , output_frame != nullptr ? m_output_frame->pixels() : nullptr)
    {

    }

    ~context_t()
    {

    }

    inline void set_output_frame(image_frame_t* output_frame)
    {
        m_output_frame = output_frame;
        void* pixels = m_output_frame ? m_output_frame->pixels() : nullptr;
        m_draw_processor.set_output_image(detail::create_frame_info(m_output_frame)
                                          , pixels);
    }
    inline const image_frame_t* output_frame() const
    {
        return m_output_frame;
    }

    inline const config_t& config() const
    {
        return m_config;
    }

    bool draw_image_frame(const image_frame_t& input_frame
                          , const draw_options_t& draw_options)
    {
        if (input_frame.is_valid())
        {
            m_draw_processor.draw_format().draw_opacity = draw_options.opacity;
            auto dst_rect = base::frame_utils::rect_from_relative(draw_options.target_rect
                                                                 , m_output_frame->size);
            if (dst_rect.is_null())
            {
                dst_rect.size = input_frame.size;
                dst_rect.fit(m_output_frame->size);
            }

            if (!dst_rect.is_null())
            {
                frame_rect_t src_rect = { 0, 0, input_frame.size.width, input_frame.size.height };
                src_rect.aspect_ratio(dst_rect);
                m_draw_processor.draw_image(dst_rect
                                            , src_rect
                                            , detail::create_frame_info(input_frame)
                                            , input_frame.pixels());

                return true;
            }
        }

        return false;
    }

    bool blackout()
    {
        if (m_output_frame->is_empty())
        {
            m_draw_processor.blackout();
            return true;
        }
        return false;
    }

    inline bool is_valid() const
    {
        return m_output_frame != nullptr
                && m_output_frame->is_valid()
                && m_output_frame->format_id == video_format_id_t::bgr32;
    }
};

//-----------------------------------------------------------------------

video_image_builder::video_image_builder(const config_t& config
                                         , image_frame_t *output_frame)
    : m_context(context_t::create(config
                                  , output_frame))
{

}

video_image_builder::~video_image_builder()
{

}

void video_image_builder::set_output_frame(image_frame_t *output_frame)
{
    m_context->set_output_frame(output_frame);
}

const image_frame_t *video_image_builder::output_frame() const
{
    return m_context->output_frame();
}

const video_image_builder::config_t &video_image_builder::config() const
{
    return m_context->config();
}

bool video_image_builder::draw_image_frame(const image_frame_t &input_frame
                                           , const draw_options_t &draw_options)
{
    return m_context->draw_image_frame(input_frame
                                       , draw_options);
}

bool video_image_builder::blackout()
{
    return m_context->blackout();
}

bool video_image_builder::is_valid() const
{
    return m_context->is_valid();
}


}
