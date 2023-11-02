#include "media_engine_impl.h"
#include "tools/ffmpeg/libav_base.h"

#include <atomic>

namespace mpl::media
{

namespace detail
{

inline void check_and_init_libav()
{
    if (!pt::ffmpeg::is_registered())
    {
        pt::ffmpeg::libav_register();
    }
}

}

struct media_engine_impl::pimpl_t
{
    using config_t = media_engine_impl::config_t;
    using u_ptr_t = media_engine_impl::pimpl_ptr_t;

    config_t            m_config;
    i_task_manager&     m_task_manager;

    std::atomic_bool    m_start;

    static u_ptr_t create(const config_t& config
                          , i_task_manager& task_manager)
    {
        detail::check_and_init_libav();
        return std::make_unique<pimpl_t>(config
                                         , task_manager);
    }

    pimpl_t(const config_t& config
            , i_task_manager& task_manager)
        : m_config(config)
        , m_task_manager(task_manager)
        , m_start(false)
    {

    }

    ~pimpl_t()
    {
        stop();
    }

    inline bool start()
    {
        bool flag  = false;
        if (m_start.compare_exchange_strong(flag, true))
        {
            if (m_task_manager.is_started())
            {
                return true;
            }

            m_start.store(false, std::memory_order_release);
        }

        return false;
    }

    inline bool stop()
    {
        bool flag = true;
        return m_start.compare_exchange_strong(flag, false);
    }

    inline bool is_started() const
    {
        return m_start.load(std::memory_order_acquire)
                && m_task_manager.is_started();
    }
};

media_engine_impl::u_ptr_t media_engine_impl::create(const config_t &config
                                                     , i_task_manager &task_manager)
{
    return std::make_unique<media_engine_impl>(config
                                               , task_manager);
}

media_engine_impl::media_engine_impl(const config_t& config
                                     , i_task_manager &task_manager)
    : m_pimpl(pimpl_t::create(config
                              , task_manager))
{

}

bool media_engine_impl::start()
{
    return m_pimpl->start();
}

bool media_engine_impl::stop()
{
    return m_pimpl->stop();
}

bool media_engine_impl::is_started() const
{
    return m_pimpl->is_started();
}


}
