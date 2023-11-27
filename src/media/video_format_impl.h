#ifndef MPL_VIDEO_FORMAT_IMPL_H
#define MPL_VIDEO_FORMAT_IMPL_H

#include "i_video_format.h"
#include "utils/option_impl.h"
#include "core/i_parametrizable.h"
#include "video_info.h"
#include <string>

namespace mpl::media
{

class video_format_impl : public i_video_format
        , public i_parametrizable
{
    video_info_t            m_video_info;
    option_impl             m_options;

public:
    using u_ptr_t = std::unique_ptr<video_format_impl>;
    using s_ptr_t = std::shared_ptr<video_format_impl>;

    static video_format_impl& undefined_video_fromat();

    static u_ptr_t create(const video_format_id_t& format_id = video_format_id_t::undefined
                          , std::int32_t width = 0
                          , std::int32_t height = 0
                          , double frame_rate = 0.0);

    static u_ptr_t create(const video_info_t& video_info);


    static u_ptr_t create(const i_video_format& other);
    static u_ptr_t create(const i_property& params);

    video_format_impl(const video_format_id_t& format_id = video_format_id_t::undefined
                      , std::int32_t width = 0
                      , std::int32_t height = 0
                      , double frame_rate = 0.0);

    video_format_impl(const video_info_t& video_info);

    video_format_impl(const i_video_format& other);
    video_format_impl(const i_property& params);

    const video_info_t& info() const;

    video_format_impl& set_format_id(const video_format_id_t& format_id);
    video_format_impl& set_width(std::int32_t width);
    video_format_impl& set_height(std::int32_t height);
    video_format_impl& set_frame_rate(double frame_rate);
    video_format_impl& set_options(option_impl&& options);
    video_format_impl& set_options(const i_option& options);
    video_format_impl& set_video_info(const video_info_t& info);
    video_format_impl& assign(const i_video_format& other);

    bool set_params(const i_property& params) override;
    bool get_params(i_property& params) const override;

    i_property::u_ptr_t get_params(const std::string& path = {}) const;

    option_impl& options();

    // i_media_format interface
public:
    media_type_t media_type() const override;
    bool is_encoded() const override;
    bool is_convertable() const override;
    i_media_format::u_ptr_t clone() const override;
    bool is_equal(const i_media_format &other) const override;
    bool is_compatible(const i_media_format &other) const override;
    const i_option &options() const override;
    bool is_valid() const override;

    // i_video_format interface
public:
    video_format_id_t format_id() const override;
    int32_t width() const override;
    int32_t height() const override;
    double frame_rate() const override;
};

}

#endif // MPL_VIDEO_FORMAT_IMPL_H