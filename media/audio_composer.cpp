#include "audio_composer.h"
#include "audio_mixer.h"
#include "audio_level.h"

#include <set>

namespace mpl::media
{

audio_composer::config_t::config_t(const sample_info_t &sample_info
                                   , std::size_t samples)
    : sample_info(sample_info)
    , samples(samples)

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

        audio_mixer         m_audio_mixer;

        audio_level         m_audio_level;
        std::size_t         m_frame_count;

    public:
        using set_t = std::multiset<compose_stream_impl*>;

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
            , m_stream_sample(m_owner.m_compose_sample.sample_info)
            , m_audio_mixer(m_stream_sample.sample_info
                            , m_stream_sample.sample_info.sample_rate / 2)
            , m_frame_count(0)

        {

        }

        ~compose_stream_impl()
        {
            m_owner.on_remove_stream(this);
        }

        inline bool is_overrun()
        {
            return m_audio_mixer.overrun();
        }

        inline bool has_enabled() const
        {
            return m_compose_options.enabled;
        }

        inline bool mix(void* data, std::size_t samples)
        {
            if (is_overrun())
            {
                m_audio_mixer.reset();
                m_audio_level.reset();

                return false;
            }

            return m_audio_mixer.read_data(data
                                          , samples
                                          , audio_mixer::mix_method_t::mix) == samples;

        }

        inline bool compose()
        {
            if (m_owner.m_compose_sample.is_valid())
            {
                m_stream_sample = m_owner.m_compose_sample;
                auto samples = m_stream_sample.samples();

                m_audio_mixer.pop_data(m_stream_sample.data()
                                       , samples
                                       , audio_mixer::mix_method_t::demix);

                m_frame_count++;

                return true;
            }

            return false;
        }

        // i_compose_stream interface
    public:
        double level() const override
        {
            return m_audio_level.level();
        }

        bool push_stream_sample(audio_sample_t &&sample) override
        {
            if (has_enabled()
                    && sample.is_valid()
                    && sample.sample_info == m_audio_mixer.sample_info())
            {
                auto data = sample.data();
                auto samples = sample.samples();
                m_audio_level.push_frame(sample.sample_info
                                         , data
                                         , samples);
                return m_audio_mixer.push_data(data
                                              , samples
                                              , m_compose_options.volume);
            }
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

        std::size_t frame_count() const override
        {
            return m_frame_count;
        }
    };


    config_t                    m_config;
    audio_sample_t              m_compose_sample;
    compose_stream_impl::set_t  m_streams;

    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<pimpl_t>(config);
    }

    pimpl_t(const config_t& config)
        : m_config(config)
        , m_compose_sample(m_config.sample_info)
    {
        m_compose_sample.resize(config.samples);
        // m_compose_sample.tune();
    }

    const audio_sample_t* compose()
    {
        if (m_compose_sample.is_valid())
        {
            m_compose_sample.clear();

            for (const auto& s : m_streams)
            {
                s->mix(m_compose_sample.data()
                       , m_compose_sample.samples());
            }

            for (const auto& s : m_streams)
            {
                s->compose();
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
