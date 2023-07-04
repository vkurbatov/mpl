#ifndef MPL_MEDIA_IMAGE_BUILDER_H
#define MPL_MEDIA_IMAGE_BUILDER_H

#include <memory>

namespace mpl::media
{

struct image_frame_t;
struct draw_options_t;

class image_builder
{
public:
    struct config_t
    {

    };

    struct context_t;
    using context_ptr_t = std::unique_ptr<context_t>;

    context_ptr_t       m_context;

public:

    image_builder(const config_t& config
                        , image_frame_t* output_frame);
    ~image_builder();

    void set_output_frame(image_frame_t* output_frame);
    const image_frame_t* output_frame() const;
    const config_t& config() const;

    bool draw_image_frame(const image_frame_t& input_frame
                          , const draw_options_t& draw_options);

    bool blackout();
    bool is_valid() const;

};

}

#endif // MPL_MEDIA_IMAGE_BUILDER_H
