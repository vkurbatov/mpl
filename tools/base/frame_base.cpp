#include "frame_base.h"
#include <algorithm>
#include <cmath>

namespace base
{

template class frame_point_base_t<std::int32_t>;
template class frame_size_base_t<std::int32_t>;
template class frame_rect_base_t<std::int32_t>;

template class frame_point_base_t<double>;
template class frame_size_base_t<double>;
template class frame_rect_base_t<double>;

template<typename T>
frame_point_base_t<T>::frame_point_base_t(T x
                                          , T y)
    : x(x)
    , y(y)
{

}

template<typename T>
bool frame_point_base_t<T>::is_null() const
{
    return x == 0 && y == 0;
}

template<typename T>
bool frame_point_base_t<T>::operator ==(const frame_point_base_t<T> &frame_point) const
{
    return x == frame_point.x
            && y == frame_point.y;
}

template<typename T>
bool frame_point_base_t<T>::operator !=(const frame_point_base_t<T> &frame_point) const
{
    return !operator==(frame_point);
}

template<typename T>
frame_point_base_t<T>& frame_point_base_t<T>::operator +=(const frame_point_base_t<T> &frame_point)
{
    x += frame_point.x;
    y += frame_point.y;

    return *this;
}

template<typename T>
T frame_point_base_t<T>::distance(const value_type &frame_point) const
{
    return std::sqrt(std::pow(frame_point.x - x, 2) + std::pow(frame_point.y - y, 2));
}

template<typename T>
frame_point_base_t<T> &frame_point_base_t<T>::operator +=(const frame_size_base_t<T> &frame_size)
{
    x += frame_size.width;
    y += frame_size.height;

    return *this;
}

template<typename T>
frame_size_base_t<T>::frame_size_base_t(T width, T height)
    : width(width)
    , height(height)
{

}

template<typename T>
std::size_t frame_size_base_t<T>::size() const
{
    return width * height;
}

template<typename T>
bool frame_size_base_t<T>::is_null() const
{
    return width == 0 || height == 0;
}

template<typename T>
bool frame_size_base_t<T>::operator ==(const frame_size_base_t<T> &frame_size) const
{
    return width == frame_size.width
            && height == frame_size.height;
}

template<typename T>
bool frame_size_base_t<T>::operator !=(const frame_size_base_t<T> &frame_size) const
{
    return !operator==(frame_size);
}

template<typename T>
frame_size_base_t<T> &frame_size_base_t<T>::operator +=(const frame_size_base_t<T> &frame_size)
{
    width += frame_size.width;
    height += frame_size.height;

    return *this;
}

template<typename T>
frame_size_base_t<T> &frame_size_base_t<T>::operator +=(const frame_point_base_t<T> &frame_point)
{
    width += frame_point.x;
    height += frame_point.y;

    return *this;
}

template<typename T>
void frame_rect_base_t<T>::aspect_ratio(const frame_rect_base_t<T> &input_rect
                                        , frame_rect_base_t<T> &output_rect)
{
    auto adjusted_width = (output_rect.size.height * input_rect.size.width) / input_rect.size.height;
    auto adjusted_height = (output_rect.size.width * input_rect.size.height) / input_rect.size.width;

    auto new_width = std::min(adjusted_width, output_rect.size.width);
    auto new_height = std::min(adjusted_height, output_rect.size.height);

    output_rect.offset.x += (output_rect.size.width - new_width) / 2;
    output_rect.offset.y += (output_rect.size.height - new_height) / 2;

    output_rect.size.width = new_width;
    output_rect.size.height = new_height;
}

template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(const frame_point_base_t<T> &offset
                                        , const frame_size_base_t<T> &size)
    : offset(offset)
    , size(size)
{

}


template<typename T>
frame_rect_base_t<T>::frame_rect_base_t(T x
                                       , T y
                                       , T width
                                       , T height)
    : frame_rect_base_t({ x, y }
                        , { width, height })
{

}


template<typename T>
bool frame_rect_base_t<T>::operator ==(const frame_rect_base_t<T> &frame_rect) const
{
    return offset == frame_rect.offset && size == frame_rect.size;
}


template<typename T>
bool frame_rect_base_t<T>::operator !=(const frame_rect_base_t<T> &frame_rect) const
{
    return !operator==(frame_rect);
}


template<typename T>
frame_rect_base_t<T> &frame_rect_base_t<T>::operator +=(const frame_size_base_t<T> &frame_size)
{
    size += frame_size;
    return *this;
}


template<typename T>
frame_rect_base_t<T> &frame_rect_base_t<T>::operator +=(const frame_point_base_t<T> &frame_point)
{
    offset += frame_point;
    return *this;
}

template<typename T>
frame_point_base_t<T> frame_rect_base_t<T>::br_point() const
{
    return { offset.x + size.width, offset.y + size.height };
}

template<typename T>
frame_point_base_t<T> frame_rect_base_t<T>::center() const
{
    return { (offset.x + size.width / 2) , (offset.y + size.height / 2) };
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::fit(const frame_size_base_t<T> &frame_size)
{
    offset.x = std::max(offset.x, static_cast<T>(0));
    offset.y = std::max(offset.y, static_cast<T>(0));
    offset.x = std::min(offset.x, frame_size.width);
    offset.y = std::min(offset.y, frame_size.height);
    if (br_point().x > frame_size.width)
    {
        size.width = frame_size.width - offset.x;
    }
    if (br_point().y > frame_size.height)
    {
        size.height = frame_size.height - offset.y;
    }
    return *this;
}

template<typename T>
frame_rect_base_t<T>& frame_rect_base_t<T>::aspect_ratio(const frame_rect_base_t<T> &rect)
{
    aspect_ratio(rect
                 , *this);
    return *this;
}

template<typename T>
bool frame_rect_base_t<T>::is_join(const frame_size_base_t<T> &frame_size) const
{
    return frame_size.width >= (offset.x + size.width)
            && frame_size.height >= (offset.y + size.height)
            && offset.x >= 0
            && offset.y >= 0;
}

template<typename T>
bool frame_rect_base_t<T>::is_null() const
{
    return offset.is_null()
            && size.width == 0 && size.height == 0;
}

namespace frame_utils
{

frame_rect_t rect_from_relative(const frame_rect_float_t &relative_rect
                                , const frame_size_t &frame_size
                                , double margin)
{
    auto real_magrin = std::max(frame_size.width, frame_size.height) * margin;
    frame_rect_t rect(frame_size.width * relative_rect.offset.x + real_magrin
                      , frame_size.height * relative_rect.offset.y + real_magrin
                      , frame_size.width * relative_rect.size.width - real_magrin * 2
                      , frame_size.height * relative_rect.size.height - real_magrin * 2);

    return rect.fit(frame_size);
}

}

}
