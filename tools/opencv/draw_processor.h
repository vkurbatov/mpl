#ifndef OCV_DRAW_PROCESSOR_H
#define OCV_DRAW_PROCESSOR_H

#include "ocv_types.h"
#include "draw_format.h"
#include "frame_info.h"
#include <memory>


namespace ocv
{

struct ocv_context_t;
using ocv_context_ptr_t = std::shared_ptr<ocv_context_t>;

class draw_processor
{
    ocv_context_ptr_t   m_context;
public:
    draw_processor(const frame_info_t& format = {}
            , void *pixels = nullptr);
    draw_format_t& draw_format();
    bool set_cutom_font(const std::string& font_path);
    void set_output_image(const frame_info_t& format
                          , void *pixels);
    void draw_text(const frame_point_t& pos
                   , const std::string& text);
    void draw_rect(const frame_rect_t& rect);
    void draw_ellipse(const frame_rect_t& rect);
    void draw_figure(const frame_rect_t& rect
                     , draw_figure_t figure = draw_figure_t::rectangle);
    void draw_fill_rect(const frame_rect_t& rect);
    void draw_image(const frame_point_t& pos
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle);
    void draw_image(const frame_rect_t& rect_to
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle);

    void draw_image(const frame_rect_t& rect_to
                    , const frame_rect_t& rect_from
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle);

    void draw_poly(const frame_point_list_t& point_list);



    frame_size_t get_text_size(const std::string& text) const;
    bool is_truefont() const;

    void display();

};

}

#endif // OCV_DRAW_PROCESSOR_H
