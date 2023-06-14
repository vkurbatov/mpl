#include "draw_processor.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>

#include <chrono>
#include <iostream>
#include <thread>
#include <cstring>

namespace ocv
{

namespace
{
    std::int32_t get_image_type(const frame_format_t& format)
    {
        switch(format)
        {
            case frame_format_t::bgr:
            case frame_format_t::rgb:
                return CV_8UC3;
            break;
            case frame_format_t::bgra:
            case frame_format_t::rgba:
                return CV_8UC4;
            break;
            default:
            {}
        }

        return 0;
    }

    cv::Scalar scalar_from_color(color_t color)
    {
        return cv::Scalar(  (color >> 24) & 0xff
                              , (color >> 16) & 0xff
                              , (color >> 8) & 0xff
                              , (color >> 0) & 0xff
                          );
        //return cv::Scalar(color);
    }
}

struct draw_processor::context_t
{
    draw_format_t                       m_draw_format;
    cv::Ptr<cv::freetype::FreeType2>    m_custom_font;
    cv::Mat                             m_output_mat;

    context_t(const frame_info_t& format
                  , void *pixels)
        : m_draw_format{}
        , m_custom_font(nullptr)
    {

    }

    ~context_t()
    {

    }

    bool set_cutom_font(const std::string &font_path)
    {
        try
        {
            if (font_path.empty())
            {
                m_custom_font.reset();
            }
            else
            {
                m_custom_font = cv::freetype::createFreeType2();
                m_custom_font->loadFontData(font_path, 0);
            }
            return true;
        }
        catch(const std::exception& e)
        {
            m_custom_font.reset();
            // TODO: log
        }

        return false;
    }

    void set_output_image(const frame_info_t& format
                          , void *pixels)
    {
        auto type = get_image_type(format.format);
        if (type != 0
                && pixels != nullptr)
        {
            m_output_mat = cv::Mat(format.size.height
                                   , format.size.width
                                   , type
                                   , pixels);
            return;

        }
    }

    void set_output_data(void *pixels)
    {
        m_output_mat.data = static_cast<u_char*>(pixels);
        return;
    }

    void draw_text(const frame_point_t& pos
                  , const std::string& text)
    {
        if (is_output_set())
        {

            if (m_custom_font != nullptr
                    && m_output_mat.channels() == 3)
            {
                try
                {
                    m_custom_font->putText(m_output_mat
                                           , text
                                           , { pos.x, pos.y }
                                           , m_draw_format.font_format.height
                                           , scalar_from_color(m_draw_format.font_color)
                                           , m_draw_format.font_format.weight
                                           , cv::LineTypes::LINE_AA
                                           , true);
                }
                catch(const std::exception& e)
                {
                    std::cout << e.what() << std::endl;
                    // log
                }
            }
            else
            {
                cv::putText(m_output_mat
                            , text
                            , { pos.x, pos.y }
                            , m_draw_format.font_format.native_font()
                            , m_draw_format.font_format.scale_font()
                            , scalar_from_color(m_draw_format.font_color)
                            , m_draw_format.font_format.weight);
            }
        }
    }

    void draw_rect(const frame_rect_t& rect)
    {
        if (is_output_set()
                && !rect.is_null())
        {
            cv::rectangle(m_output_mat
                          , { rect.offset.x, rect.offset.y, rect.size.width, rect.size.height }
                          , scalar_from_color(m_draw_format.pen_color)
                          , std::min(m_draw_format.line_weight, 10));
        }
    }

    void draw_ellipse(const frame_rect_t& rect)
    {
        if (is_output_set()
                && !rect.is_null())
        {
            cv::ellipse(m_output_mat
                          , { rect.offset.x + rect.size.width / 2, rect.offset.y + rect.size.height / 2 }
                          , { rect.size.width / 2, rect.size.height / 2 }
                          , 0, 0, 360
                          , scalar_from_color(m_draw_format.pen_color)
                          , std::min(m_draw_format.line_weight, 10));
        }
    }

    void draw_figure(const frame_rect_t &rect, draw_figure_t figure)
    {
        switch(figure)
        {
            case draw_figure_t::rectangle:
                draw_rect(rect);
            break;
            case draw_figure_t::ellipse:
                draw_ellipse(rect);
            break;
        }
    }

    void draw_fill_rect(const frame_rect_t& rect)
    {
        if (is_output_set()
                && !rect.is_null())
        {
            const frame_point_list_t points = { rect.offset
                                                , { rect.br_point().x, rect.offset.y }
                                                , rect.br_point()
                                                , { rect.offset.x, rect.br_point().y }};

            draw_poly(points);
            draw_rect(rect);

        }

    }

    void transparent_overlay(const cv::Mat& input
                             , cv::Mat& output
                             , double opacity = 1.0
                             , cv::Mat mask = {})
    {

        if (opacity == 1.0
                && mask.empty()
                && input.channels() != 4)
        {
            input.copyTo(output);
        }
        else
        {
            if (input.channels() == 4)
            {
                std::vector<cv::Mat> mat_channels;
                cv::split(input, mat_channels);
                if (!mask.empty())
                {
                    cv::bitwise_and(mat_channels[3], mask, mask);
                }
                else
                {
                    mask = mat_channels[3];
                }
            }

            if (opacity == 1.0)
            {
                input.copyTo(output, mask);
            }
            else
            {
                if (mask.empty())
                {
                    cv::addWeighted(input
                                    , opacity
                                    , output
                                    , 1.0 - opacity
                                    , 0.0
                                    , output);
                }
                else
                {
                    cv::Mat tmp;
                    cv::addWeighted(input
                                    , opacity
                                    , output
                                    , 1.0 - opacity
                                    , 0.0
                                    , tmp);
                    tmp.copyTo(output
                               , mask);
                }

            }
        }

    }

    cv::Mat create_mask(std::int32_t cols
                        , std::int32_t rows
                        , draw_figure_t figure = draw_figure_t::rectangle)
    {
        cv::Mat mask;

        switch(figure)
        {
            case draw_figure_t::ellipse:
            {
                mask = cv::Mat(rows, cols, CV_8UC1, cv::Scalar(0));
                cv::ellipse(mask
                            , { cols / 2, rows / 2 }
                            , { cols / 2,  rows / 2}
                            , 0, 0, 360
                            , cv::Scalar(255)
                            , -1);
            }
            break;
            default:
            {

            }
        }

        return mask;
    }

    void draw_matrix(const cv::Mat& input
                     , cv::Mat& output
                     , double opacity
                     , draw_figure_t figure = draw_figure_t::rectangle)
    {
        transparent_overlay(input
                            , output
                            , opacity
                            , create_mask(output.cols, output.rows, figure));
    }

    void draw_image(const frame_point_t& pos
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle)
    {
        if (is_output_set())
        {
            auto type = get_image_type(format.format);
            if (type != 0
                    && pixels != nullptr)
            {
                cv::Mat input_matrix(format.size.height
                                     , format.size.width
                                     , type
                                     , const_cast<void*>(pixels));

                auto output = m_output_mat({ pos.x, pos.y, pos.x + input_matrix.rows, pos.y + input_matrix.cols });

                draw_matrix(input_matrix
                            , output
                            , m_draw_format.draw_opacity
                            , figure);
            }
        }
    }

    void draw_image(const frame_rect_t& rect_to
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle)
    {
        if (is_output_set())
        {
            auto type = get_image_type(format.format);
            if (type != 0
                    && pixels != nullptr)
            {
                cv::Mat input_matrix(format.size.height
                                     , format.size.width
                                     , type
                                     , const_cast<void*>(pixels));

                cv::Mat scale_matrix;

                cv::resize(input_matrix
                           , scale_matrix
                           , { rect_to.size.width, rect_to.size.height });

                auto output = m_output_mat({rect_to.offset.x, rect_to.offset.y, rect_to.size.width, rect_to.size.height});

                draw_matrix(scale_matrix
                            , output
                            , m_draw_format.draw_opacity
                            , figure);
            }
        }
    }


    void draw_image(const frame_rect_t& rect_to
                    , const frame_rect_t& rect_from
                    , const frame_info_t& format
                    , const void *pixels
                    , draw_figure_t figure = draw_figure_t::rectangle)
    {
        if (is_output_set())
        {
            auto type = get_image_type(format.format);
            if (type != 0
                    && pixels != nullptr)
            {

                cv::Mat input_matrix(format.size.height
                                     , format.size.width
                                     , type
                                     , const_cast<void*>(pixels));


                input_matrix = input_matrix({rect_from.offset.x, rect_from.offset.y, rect_from.size.width, rect_from.size.height});

                cv::Mat scale_matrix;

                cv::resize(input_matrix
                           , scale_matrix
                           , { rect_to.size.width, rect_to.size.height });

                auto output = m_output_mat({rect_to.offset.x, rect_to.offset.y, rect_to.size.width, rect_to.size.height});

                draw_matrix(scale_matrix
                            , output
                            , m_draw_format.draw_opacity
                            , figure);
            }
        }
    }

    void draw_poly(const frame_point_list_t& point_list)
    {
        if (is_output_set())
        {
            std::vector<cv::Point> points;
            for (const auto& p : point_list)
            {
                points.emplace_back(p.x, p.y);
            }
            cv::fillConvexPoly(m_output_mat
                               , points
                               , scalar_from_color(m_draw_format.fill_color));
        }
    }

    void blackout()
    {
        if (auto pixels = m_output_mat.data)
        {
            std::memset(pixels, 0, m_output_mat.size.dims());
        }
    }

    frame_size_t get_text_size(const std::string& text) const
    {
        std::int32_t lines = 0;

        if (m_custom_font != nullptr)
        {
            auto cv_size = m_custom_font->getTextSize(text
                                                     , m_draw_format.font_format.height
                                                     , m_draw_format.font_format.weight
                                                     , &lines);
            return { cv_size.width, cv_size.height };

        }
        return m_draw_format.font_format.text_size(text);
    }


    void display()
    {

        if (is_output_set())
        {
            /*
            cv::imshow("Test show", m_output_mat);
            cv::waitKey(0);
            cv::destroyAllWindows();*/
        }
    }

    bool is_output_set() const
    {
        return m_output_mat.data != nullptr;
    }

};

draw_processor::draw_processor(const frame_info_t& format
                               , void *pixels)
    : m_context(std::make_unique<context_t>(format
                                            , pixels))
{

}

draw_processor::~draw_processor()
{

}

draw_format_t &draw_processor::draw_format()
{
    return m_context->m_draw_format;
}

bool draw_processor::set_cutom_font(const std::string &font_path)
{
    return m_context->set_cutom_font(font_path);
}

void draw_processor::set_output_image(const frame_info_t &format
                                      , void *pixels)
{
    m_context->set_output_image(format
                                , pixels);
}

void draw_processor::set_output_data(void *pixels)
{
    m_context->set_output_data(pixels);
}

void draw_processor::draw_text(const frame_point_t &pos
                              , const std::string &text)
{
    m_context->draw_text(pos
                         , text);
}

void draw_processor::draw_rect(const frame_rect_t &rect)
{
    m_context->draw_rect(rect);
}

void draw_processor::draw_ellipse(const frame_rect_t &rect)
{
    m_context->draw_ellipse(rect);
}

void draw_processor::draw_figure(const frame_rect_t &rect, draw_figure_t figure)
{
    m_context->draw_figure(rect, figure);
}

void draw_processor::draw_fill_rect(const frame_rect_t &rect)
{
    m_context->draw_fill_rect(rect);
}

void draw_processor::draw_image(const frame_point_t &pos
                                , const frame_info_t &format
                                , const void *pixels
                                , draw_figure_t figure)
{
    m_context->draw_image(pos
                          , format
                          , pixels
                          , figure);
}

void draw_processor::draw_image(const frame_rect_t &rect_to
                                , const frame_info_t &format
                                , const void *pixels
                                , draw_figure_t figure)
{
    m_context->draw_image(rect_to
                          , format
                          , pixels
                          , figure);
}

void draw_processor::draw_image(const frame_rect_t &rect_to
                                , const frame_rect_t &rect_from
                                , const frame_info_t &format
                                , const void *pixels
                                , draw_figure_t figure)
{
    m_context->draw_image(rect_to
                          , rect_from
                          , format
                          , pixels
                          , figure);
}

void draw_processor::draw_poly(const frame_point_list_t &point_list)
{
    m_context->draw_poly(point_list);
}

void draw_processor::blackout()
{
    m_context->blackout();
}

frame_size_t draw_processor::get_text_size(const std::string& text) const
{
    return m_context->get_text_size(text);
}

bool draw_processor::is_truefont() const
{
    return m_context->m_custom_font != nullptr;
}

void draw_processor::display()
{
    m_context->display();
}

}
