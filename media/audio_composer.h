#ifndef MPL_AUDIO_COMPOSER_H
#define MPL_AUDIO_COMPOSER_H

#include "audio_sample.h"

namespace mpl::media
{

class audio_composer
{
public:

    struct config_t
    {
        sample_info_t   sample_info;
        std::size_t     samples;
        config_t(const sample_info_t& sample_info = {}
                 , std::size_t samples = 0);
    };

    struct compose_options_t
    {
        double              volume;
        bool                enabled;

        compose_options_t(double volume
                          , bool enabled = true);

    };

    class i_compose_stream
    {
    public:
        using u_ptr_t = std::unique_ptr<i_compose_stream>;
        using s_ptr_t = std::shared_ptr<i_compose_stream>;
        using w_ptr_t = std::weak_ptr<i_compose_stream>;

        virtual ~i_compose_stream() = default;

        virtual double level() const = 0;
        virtual bool push_stream_sample(audio_sample_t&& sample) = 0;
        virtual const compose_options_t& options() const = 0;
        virtual compose_options_t& options() = 0;
        virtual const audio_sample_t* compose_sample() const = 0;
        virtual std::size_t frame_count() const = 0;

    };

private:

    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;

public:

    audio_composer(const config_t& config);
    ~audio_composer();

    const audio_sample_t* compose();

    i_compose_stream::s_ptr_t add_stream(const compose_options_t& options);
};

}

#endif // MPL_AUDIO_COMPOSER_H
