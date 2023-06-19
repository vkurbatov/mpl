#ifndef FRAME_BASE_H
#define FRAME_BASE_H

#include <cstdint>

namespace base
{

template<typename T>
struct frame_size_base_t; //fwd


template<typename T>
struct frame_point_base_t
{
    using value_type = frame_point_base_t<T>;
    T   x;
    T   y;

    frame_point_base_t(T x = {}
                        , T y = {});

    bool is_null() const;

    bool operator ==(const value_type& frame_point) const;
    bool operator !=(const value_type& frame_point) const;

    value_type& operator +=(const value_type& frame_point);
    value_type& operator +=(const frame_size_base_t<T>& frame_size);

    T distance(const value_type& frame_point) const;
};

template<typename T>
struct frame_size_base_t
{
    using value_type = frame_size_base_t<T>;

    T   width;
    T   height;

    frame_size_base_t(T width = {}
                        , T height = {});

    std::size_t size() const;
    bool is_null() const;
    bool operator ==(const value_type& frame_size) const;
    bool operator !=(const value_type& frame_size) const;

    value_type& operator +=(const value_type& frame_size);
    value_type& operator +=(const frame_point_base_t<T>& frame_point);

};

template<typename T>
struct frame_rect_base_t
{
    using value_type = frame_rect_base_t<T>;

    frame_point_base_t<T>   offset;
    frame_size_base_t<T>    size;

    static void aspect_ratio(const value_type& input_rect
                             , value_type& output_rect);

    frame_rect_base_t(const frame_point_base_t<T>& offset = { }
                      , const frame_size_base_t<T>& size = { });

    frame_rect_base_t(T x
                     , T y
                     , T width
                     , T height);

    bool operator ==(const value_type& frame_rect) const;
    bool operator !=(const value_type& frame_rect) const;

    value_type& operator +=(const frame_size_base_t<T>& frame_size);
    value_type& operator +=(const frame_point_base_t<T>& frame_point);

    frame_point_base_t<T> br_point() const;
    frame_point_base_t<T> center() const;
    frame_rect_base_t<T>& aspect_ratio(const value_type& rect);
    frame_rect_base_t<T>& fit(const frame_size_base_t<T>& frame_size);
    bool is_join(const frame_size_base_t<T>& frame_size) const;
    bool is_null() const;
};

typedef frame_point_base_t<std::int32_t> frame_point_t;
typedef frame_size_base_t<std::int32_t> frame_size_t;
typedef frame_rect_base_t<std::int32_t> frame_rect_t;

typedef frame_point_base_t<double> frame_point_float_t;
typedef frame_size_base_t<double> frame_size_float_t;
typedef frame_rect_base_t<double> frame_rect_float_t;

namespace frame_utils
{
    frame_rect_t rect_from_relative(const frame_rect_float_t& relative_rect
                                    , const frame_size_t& frame_size
                                    , double margin = 0.0);
}

}

#endif // FRAME_BASE_H
