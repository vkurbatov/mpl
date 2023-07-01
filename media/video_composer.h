#ifndef MPL_VIDEO_COMPOSER_H
#define MPL_VIDEO_COMPOSER_H

#include "draw_options.h"
#include "image_frame.h"
#include <memory>


namespace mpl::media
{

class video_composer
{
public:

    struct config_t
    {
        image_info_t    frame_info;
        config_t(const image_info_t& frame_info);
    };

    struct compose_options_t
    {
        draw_options_t      draw_options;
        std::int32_t        order;
        bool                enabled;

        compose_options_t(const draw_options_t& draw_options = {}
                          , std::int32_t order = 0
                          , bool enabled = true);

    };

    class i_compose_stream
    {
    public:
        using u_ptr_t = std::unique_ptr<i_compose_stream>;
        using s_ptr_t = std::shared_ptr<i_compose_stream>;
        using w_ptr_t = std::weak_ptr<i_compose_stream>;

        virtual ~i_compose_stream() = default;

        virtual bool push_stream_image(image_frame_t&& input_image) = 0;
        virtual const compose_options_t& options() const = 0;
        virtual compose_options_t& options() = 0;
        virtual const image_frame_t* compose_image() const = 0;
        virtual std::size_t frame_count() const = 0;
    };

private:

    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;

public:

    video_composer(const config_t& config);
    ~video_composer();

    const image_frame_t* compose();

    i_compose_stream::s_ptr_t add_stream(const compose_options_t& options);

};

}

#endif // MPL_VIDEO_COMPOSER_H
