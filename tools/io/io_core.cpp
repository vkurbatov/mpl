#include "io_core.h"

#include <boost/asio/io_context.hpp>

#include <thread>
#include <atomic>

namespace io
{

struct io_core::pimpl_t
{
    using u_ptr_t = io_core::pimpl_ptr_t;
    using config_t = io_core::config_t;
    using io_context_t = boost::asio::io_context;
    using thread_array_t = std::vector<std::thread>;

    config_t                m_config;
    io_context_t            m_io_context;

    thread_array_t          m_threads;
    std::atomic_bool        m_running;


    static u_ptr_t create(const config_t& config)
    {
        return std::make_unique<pimpl_t>(config);
    }

    pimpl_t(const config_t& config)
        : m_config(config)
        , m_io_context(m_config.total_workers())
        , m_running(false)
    {

    }

    ~pimpl_t()
    {
        stop();
    }

    bool run()
    {
        bool flag = false;
        if (m_running.compare_exchange_strong(flag, true))
        {
            for (std::size_t i = 0; i < m_config.total_workers(); i++)
            {
                m_threads.emplace_back([&, i] { worker_proc(i); });
            }
            return true;
        }

        return false;
    }

    void worker_proc(std::size_t worker_id)
    {
        boost::asio::io_context::work work(m_io_context);
        work.get_io_context().run();
    }

    bool stop()
    {
        bool flag = true;
        if (m_running.compare_exchange_strong(flag, false))
        {
            m_io_context.stop();
            for (auto& t : m_threads)
            {
                if (t.joinable())
                {
                    t.join();
                }
            }
            m_threads.clear();

            return true;
        }

        return false;
    }

    bool is_running() const
    {
        return m_running.load(std::memory_order_consume);
    }

    void post(const executor_handler_t &executor)
    {
        if (is_running())
        {
            m_io_context.post(executor);
        }
    }

};

io_core::config_t::config_t(std::size_t max_workers)
    : max_workers(max_workers)
{

}

std::size_t io_core::config_t::total_workers() const
{
    return max_workers > 0
            ? max_workers
            : std::thread::hardware_concurrency();
}

io_core &io_core::get_instance()
{
    static io_core single_core;
    return single_core;
}

io_core::u_ptr_t io_core::create(const config_t &config)
{
    return std::make_unique<io_core>(config);
}

io_core::io_core(const config_t &config)
    : m_pimpl(pimpl_t::create(config))
{

}

io_core::~io_core()
{

}

const io_core::config_t &io_core::config() const
{
    return m_pimpl->m_config;
}

bool io_core::run()
{
    return m_pimpl->run();
}

bool io_core::stop()
{
    return m_pimpl->stop();
}

bool io_core::is_running() const
{
    return m_pimpl->is_running();
}

void io_core::post(const executor_handler_t &executor)
{
    return m_pimpl->post(executor);
}

bool io_core::is_valid() const
{
    return m_pimpl != nullptr;
}

template<>
boost::asio::io_context &io_core::get() const
{
    return m_pimpl->m_io_context;
}

}
