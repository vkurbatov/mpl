#include "image.h"

#include "ocv_utils.h"

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

namespace ocv
{

namespace detail
{

std::int32_t get_cvt_code(frame_format_t target_format)
{
    switch(target_format)
    {
        case frame_format_t::bgra:
            return cv::COLOR_BGR2BGRA;
        break;
        case frame_format_t::rgb:
            return cv::COLOR_BGR2RGB;
        break;
        case frame_format_t::rgba:
            return cv::COLOR_BGR2RGBA;
        break;
    }

    return -1;
}

}

image_t::image_t(const frame_info_t &info
                 , raw_data_t &&data)
    : info(info)
    , data(std::move(data))
{

}

image_t::image_t(const frame_info_t &info
                 , const raw_data_t &data)
    : info(info)
    , data(data)
{

}

bool image_t::is_valid() const
{
    return info.is_valid()
            && info.frame_size() == data.size();
}

bool image_t::is_empty() const
{
    return data.empty();
}

bool image_t::load(const std::string &file_path
                   , frame_format_t format)
{
    auto img = cv::imread(file_path, cv::IMREAD_COLOR);
    if (!img.empty())
    {
        info.format = format;
        info.size.width = img.cols;
        info.size.height = img.rows;

        auto cvt = detail::get_cvt_code(format);
        if (cvt != -1)
        {
            data.resize(info.frame_size());

            cv::Mat output(info.size.height
                           , info.size.width
                           , utils::get_format_info(info.format).type
                           , data.data());
            cv::cvtColor(img
                         , output
                         , cvt);
        }
        else
        {
            data.assign(img.data
                        , img.data + img.channels() * img.cols * img.rows);
        }

        return true;
    }
    return false;
}

bool image_t::resize(const frame_size_t &new_size)
{
    if (is_valid())
    {
        if (new_size != info.size)
        {
            cv::Mat input(info.size.height
                          , info.size.width
                          , utils::get_format_info(info.format).type
                          , data.data());

            info.size = new_size;

            raw_data_t raw(info.frame_size());
            cv::Mat output(info.size.height
                          , info.size.width
                          , utils::get_format_info(info.format).type
                          , raw.data());

            input.copyTo(output);
            data = std::move(raw);
        }

        return true;
    }

    return false;
}

}
