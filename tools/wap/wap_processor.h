#ifndef WAP_PROCESSOR_H
#define WAP_PROCESSOR_H

#include <memory>
#include "wap_base.h"

namespace wap
{

class wap_processor
{
public:
    struct config_t
    {
        sample_format_t     format;

        config_t(const sample_format_t& format = {});

        bool is_valid() const;
    };

private:
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<wap_processor>;

    pimpl_ptr_t     m_pimpl;

public:
    wap_processor(const config_t& config);
    ~wap_processor();

    bool push_playback(const void* data
                       , std::size_t samples);
    bool push_record(const void* data
                     , std::size_t samples);

    bool pop_playback(sample_t& sample);

    void reset();
};

}

#endif // WAP_PROCESSOR_H
