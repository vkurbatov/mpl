#include "video_composer.h"
#include "image_builder.h"

#include <algorithm>
#include <set>

namespace mpl::media
{

namespace detail
{

double animation(double lhs, double rhs, double k, double min_delta)
{
    if (k <= 0.0 || k >= 1.0)
    {
        return rhs;
    }

    auto delta = rhs - lhs;
    if (std::abs(delta) < min_delta)
    {
        return rhs;
    }

    return lhs + delta * k;
}

void animation(relative_frame_rect_t& dst_rect
               , const relative_frame_rect_t& src_rect
               , double k
               , double min_delta)
{
    dst_rect.offset.x = animation(dst_rect.offset.x, src_rect.offset.x, k, min_delta);
    dst_rect.offset.y = animation(dst_rect.offset.y, src_rect.offset.y, k, min_delta);
    dst_rect.size.width = animation(dst_rect.size.width, src_rect.size.width, k, min_delta);
    dst_rect.size.height = animation(dst_rect.size.height, src_rect.size.height, k, min_delta);
}

}

video_composer::config_t::config_t(const video_info_t &frame_info)
    : frame_info(frame_info)
{

}

video_composer::compose_options_t::compose_options_t(const draw_options_t &draw_options
                                                     , int32_t order
                                                     , double animation
                                                     , bool enabled)
    : draw_options(draw_options)
    , order(order)
    , animation(animation)
    , enabled(enabled)
{

}

struct video_composer::pimpl_t
{
    using config_t = video_composer::config_t;
    using u_ptr_t = video_composer::pimpl_ptr_t;
    using compose_options_t = video_composer::compose_options_t;

    class compose_stream_impl : public i_compose_stream
    {
        using u_ptr_t = std::unique_ptr<compose_stream_impl>;
        using s_ptr_t = std::shared_ptr<compose_stream_impl>;
        using w_ptr_t = std::weak_ptr<compose_stream_impl>;

        pimpl_t&                m_owner;
        compose_options_t       m_compose_options;
        image_frame_t           m_stream_image;

        relative_frame_rect_t   m_rect;

        image_builder           m_image_builder;

        std::size_t             m_frame_count;
        std::size_t             m_id;

    public:

        using array_ptr_t = std::vector<compose_stream_impl*>;
        struct comparator_t
        {
            static comparator_t& get_instance()
            {
                static comparator_t instance;
                return instance;
            }

            bool operator() (const compose_stream_impl* lhs, const compose_stream_impl* rhs) const
            {
                return *lhs < *rhs;
            }
        };

        using set_t = std::set<compose_stream_impl*>;

        static s_ptr_t create(pimpl_t& owner
                              , const compose_options_t& compose_options
                              , std::size_t id)
        {
            return std::make_shared<compose_stream_impl>(owner
                                                         , compose_options
                                                         , id);
        }

        compose_stream_impl(pimpl_t& owner
                            , const compose_options_t& compose_options
                            , std::size_t id)
            : m_owner(owner)
            , m_compose_options(compose_options)
            , m_rect(m_compose_options.draw_options.target_rect)
            , m_image_builder({}
                              , &m_owner.m_compose_image)
            , m_frame_count(0)
            , m_id(id)
        {

        }

        ~compose_stream_impl()
        {
            m_owner.on_remove_stream(this);
        }

        inline bool has_enabled() const
        {
            return m_compose_options.enabled;
        }


        inline bool compose()
        {
            if (m_stream_image.is_valid())
            {
                m_frame_count++;

                detail::animation(m_rect
                                  , m_compose_options.draw_options.target_rect
                                  , m_compose_options.animation
                                  , 0.001);
                bool is_animation = m_rect != m_compose_options.draw_options.target_rect;

                draw_options_t animation_options;

                if (is_animation)
                {
                    animation_options = m_compose_options.draw_options;
                    animation_options.target_rect = m_rect;
                }

                return m_image_builder.draw_image_frame(m_stream_image
                                                        , is_animation
                                                            ? animation_options
                                                            : m_compose_options.draw_options);
            }

            return false;
        }

        inline bool operator < (const compose_stream_impl& other) const
        {
            return m_compose_options.order < other.m_compose_options.order
                    || (m_compose_options.order == other.m_compose_options.order
                        && m_id < other.m_id);
        }

        // i_compose_stream interface
    public:
        inline bool push_stream_image(image_frame_t &&input_image) override
        {
            m_stream_image = std::move(input_image);
            return m_stream_image.is_valid();
        }

        inline const compose_options_t &options() const override
        {
            return m_compose_options;
        }

        inline compose_options_t &options() override
        {
            return m_compose_options;
        }

        inline const image_frame_t *compose_image() const override
        {
            return &m_owner.m_compose_image;
        }

        std::size_t frame_count() const override
        {
            return m_frame_count;
        }
    };

    config_t                    m_config;
    image_frame_t               m_compose_image;
    compose_stream_impl::set_t  m_streams;

    std::size_t                 m_ids;


    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<pimpl_t>(config);
    }

    pimpl_t(const config_t& config)
        : m_config(config)
        , m_compose_image(m_config.frame_info)
        , m_ids(0)
    {
        m_compose_image.tune();
    }

    const image_frame_t* compose()
    {
        if (m_compose_image.is_valid())
        {
            m_compose_image.blackout();

            for (const auto& s : active_streams())
            {
                s->compose();
            }

            return &m_compose_image;
        }

        return nullptr;
    }

    compose_stream_impl::array_ptr_t active_streams() const
    {
        compose_stream_impl::array_ptr_t streams;

        for (auto s : m_streams)
        {
            if (s->has_enabled())
            {
                streams.emplace_back(s);
            }
        }

        std::sort(streams.begin(), streams.end(), compose_stream_impl::comparator_t::get_instance());

        return streams;
    }

    video_composer::i_compose_stream::s_ptr_t add_stream(const video_composer::compose_options_t &options)
    {
        if (auto stream = compose_stream_impl::create(*this
                                                      , options
                                                      , m_ids++))
        {
            m_streams.insert(stream.get());
            return stream;
        }

        return nullptr;
    }

    void on_remove_stream(compose_stream_impl* stream)
    {
        m_streams.erase(stream);
        //
    }
};

video_composer::video_composer(const config_t &config)
    : m_pimpl(pimpl_t::create(config))
{

}

video_composer::~video_composer()
{

}

const image_frame_t *video_composer::compose()
{
    return m_pimpl->compose();
}

video_composer::i_compose_stream::s_ptr_t video_composer::add_stream(const compose_options_t &options)
{
    return m_pimpl->add_stream(options);
}


}
