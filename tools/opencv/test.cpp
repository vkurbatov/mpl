#include "test.h"
#include "draw_processor.h"
#include <opencv2/imgproc.hpp>
#include <opencv2/freetype.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

namespace pt::ocv
{

void transparent_overlay(cv::Mat& src, cv::Mat& overlay)
{
    auto channels = src.channels();
    std::int32_t overlay_step = overlay.step;
    std::int32_t src_step = src.step;

    for (std::int32_t y = 0; y < overlay.rows; y++)
    {
        auto overlay_data = overlay.data + y * overlay_step;
        auto src_data = src.data + y * src_step;

        for (std::int32_t x = 0; x < overlay.cols; x++)
        {
            double alpha = static_cast<double>(overlay_data[3]) / 255.0;

            if (alpha > 0.0)
            {
                for (std::int32_t c = 0; c < channels; c++)
                {
                    src_data[c] = src_data[c] * (1.0 - alpha) + overlay_data[c] * alpha;
                }
            }
            overlay_data += channels;
            src_data += channels;
        }
    }
}

void test1()
{

    // Read the images
    cv::Mat foreground = cv::imread("/home/user/test5.png", cv::IMREAD_UNCHANGED);
    cv::Mat background = cv::imread("/home/user/test6.png", cv::IMREAD_UNCHANGED);

    // auto cv_type = CV_8UC4;
    // auto bk_type = background.type();

    //foreground.convertTo(foreground, CV_8UC4);
    //background.convertTo(background, CV_8UC4);

    std::vector<cv::Mat> rgba;
    cv::split(background, rgba);
/*
    // Normalize the alpha mask to keep intensity between 0 and 1
    alpha.convertTo(alpha, CV_32SC4, 1.0/255);
*/
    // Storage for output image
    cv::Mat ouImage = cv::Mat::zeros(foreground.size(), foreground.type());

    cv::Mat cut = background({0, 0, foreground.cols, foreground.rows});


    transparent_overlay(cut, foreground);
    //cv::add(foreground, cut, cut);
/*
    // Multiply the foreground with the alpha matte
    cv::multiply(alpha, foreground, foreground);

    // Multiply the background with ( 1 - alpha )
    cv::multiply(cv::Scalar::all(1.0)-alpha, background, background);
*/
    // Add the masked foreground and background.
    cv::add(foreground, cut, ouImage);

    // Display image
    cv::imshow("alpha blended image", background);
    cv::waitKey(0);
}

void test2()
{
    frame_info_t   frame_info(pt::ocv::frame_format_t::bgra
                              , { 600, 600 });
    frame_data_t   frame_data(frame_info.frame_size(), 0);

    draw_processor  processor;
    processor.set_output_image(frame_info
                               , frame_data.data());


    processor.draw_format().pen_color = 0x0000ff00;
    processor.draw_format().fill_color = 0x00ff0000;
    processor.draw_format().font_color = 0xff000000;
    processor.draw_format().font_format.height = 12;
    processor.draw_format().draw_opacity = 0.5;
    processor.draw_fill_rect({ 10, 10, 580, 580 });
    processor.draw_text({100, 100}, "Vasiliy");

    auto cpy_frame = frame_data;

    /*processor.draw_image(frame_point_t{ 0, 0 }
                         , frame_info
                         , cpy_frame.data());*/

    processor.draw_image({ 100, 100, 400, 400 }
                         , { 5, 5, 590, 590 }
                         , frame_info
                         , cpy_frame.data());

    processor.display();
}
void test3()
{
    // Read the images
    cv::Mat foreground = cv::imread("/home/user/test5.png", cv::IMREAD_UNCHANGED);
    cv::Mat background = cv::imread("/home/user/test6.png", cv::IMREAD_UNCHANGED);

    cv::Rect rect((background.rows - foreground.rows) / 2, (background.cols - foreground.rows) / 2, foreground.rows, foreground.cols);

    auto cut = background(rect);

    cv::Mat mask(foreground.rows, foreground.cols, CV_8UC1, cv::Scalar(0));

    //cv::circle(mask, {foreground.rows / 2, foreground.cols / 2 }, foreground.rows / 2, cv::Scalar(255), -1);
    cv::ellipse(mask
                , { foreground.rows / 2, foreground.cols / 2 }
                , { foreground.rows / 2,  foreground.cols / 2}
                , 0, 90, 360
                , cv::Scalar(255)
                , -1);

    for (auto x = 0; x < mask.rows; x++)
    {
        for (auto y = 0; y < mask.cols; y++)
        {
            int data = mask.data[x * mask.rows + y] != 0;
            std::cout << data;
        }
        std::cout << std::endl;
    }

    auto inverse_mask = mask.clone();
    inverse_mask.inv();

    cv::subtract(cut, cut, cut, inverse_mask);
    cv::add(foreground, cut, cut, mask);


    /*
    auto inverse_mask = mask.clone();
    inverse_mask.inv();
    cv::Mat crop_image(foreground.size(), foreground.type(), cv::Scalar::all(0));
    cv::bitwise_and(foreground, foreground, crop_image, mask);

    cv::add(cut, crop_image, cut);*/

    //cv::circle()

    //cv::add(foreground, cut, cut);
    //foreground.copyTo(cut);

    // Display image
    cv::imshow("alpha blended image", background);
    cv::waitKey(0);
}

void test()
{
    test3();
}

}
