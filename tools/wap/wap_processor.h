#ifndef WAP_PROCESSOR_H
#define WAP_PROCESSOR_H

#include <memory>
#include "wap_base.h"

namespace pt::wap
{

class wap_processor
{
public:
    struct config_t
    {
        sample_format_t         format;
        processing_config_t     processing_config;

        config_t(const sample_format_t& format = {}
                , const processing_config_t& processing_config = {});

        bool is_valid() const;
    };

private:
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;

public:
    wap_processor(const config_t& config);
    ~wap_processor();

    const config_t& config() const;
    bool set_config(const config_t& config);

    bool push_playback(const void* data
                       , std::size_t samples);
    bool push_capture(const void* data
                     , std::size_t samples);

    bool pop_result(sample_t& sample);

    std::size_t get_stream_delay_ms() const;

    bool open();
    bool close();

    bool is_open() const;
};

}

#endif // WAP_PROCESSOR_H
