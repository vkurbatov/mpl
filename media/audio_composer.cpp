#include "audio_composer.h"
#include "audio_mixer.h"
#include "audio_level.h"

#include <set>

namespace mpl::media
{

audio_composer::config_t::config_t(const sample_info_t &sample_info)
    : sample_info(sample_info)

{

}

audio_composer::compose_options_t::compose_options_t(double volume
                                                     , bool enabled)
    : volume(volume)
    , enabled(enabled)
{

}

struct audio_composer::pimpl_t
{
    using config_t = audio_composer::config_t;
    using u_ptr_t = audio_composer::pimpl_ptr_t;
    using compose_options_t = audio_composer::compose_options_t;

    class compose_stream_impl : public i_compose_stream
    {
        using u_ptr_t = std::unique_ptr<compose_stream_impl>;
        using s_ptr_t = std::shared_ptr<compose_stream_impl>;
        using w_ptr_t = std::weak_ptr<compose_stream_impl>;

        pimpl_t&            m_owner;
        compose_options_t   m_compose_options;
        audio_sample_t      m_stream_sample;

    public:
        using set_t = std::set<compose_stream_impl*>;

        static s_ptr_t create(pimpl_t& owner
                              , const compose_options_t& compose_options)
        {
            return std::make_shared<compose_stream_impl>(owner
                                                         , compose_options);
        }

        compose_stream_impl(pimpl_t& owner
                            , const compose_options_t& compose_options)
            : m_owner(owner)
            , m_compose_options(compose_options)
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
            if (m_stream_sample.is_valid())
            {
                /*
                return m_image_builder.draw_image_frame(m_stream_image
                                                        , m_compose_options.draw_options);*/
            }

            return false;
        }

        // i_compose_stream interface
    public:
        double level() const override
        {
            return 0;
        }

        bool push_stream_sample(audio_sample_t &&sample) override
        {
            return false;
        }

        const compose_options_t &options() const override
        {
            return m_compose_options;
        }

        compose_options_t &options() override
        {
            return m_compose_options;
        }

        const audio_sample_t *compose_sample() const override
        {
            return &m_stream_sample;
        }
    };


    config_t                    m_config;
    audio_sample_t              m_compose_sample;
    compose_stream_impl::set_t  m_streams;

    static u_ptr_t create(const config_t& config)
    {
        return nullptr;
    }

    pimpl_t(const config_t& config)
        : m_config(config)
        , m_compose_sample(m_config.sample_info)
    {
        // m_compose_sample.tune();
    }

    const audio_sample_t* compose()
    {
        if (m_compose_sample.is_valid())
        {

            // m_compose_image.blackout();

            for (const auto& s : m_streams)
            {
                if (s->has_enabled())
                {
                    s->compose();
                }
            }

            return &m_compose_sample;
        }

        return nullptr;
    }

    audio_composer::i_compose_stream::s_ptr_t add_stream(const audio_composer::compose_options_t &options)
    {
        if (auto stream = compose_stream_impl::create(*this
                                                      , options))
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

audio_composer::audio_composer(const config_t &config)
    : m_pimpl(pimpl_t::create(config))
{

}

audio_composer::~audio_composer()
{

}

const audio_sample_t* audio_composer::compose()
{
    return m_pimpl->compose();
}

audio_composer::i_compose_stream::s_ptr_t audio_composer::add_stream(const compose_options_t &options)
{
    return m_pimpl->add_stream(options);
}

}
