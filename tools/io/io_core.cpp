#include "io_core.h"

#include <boost/asio/io_context.hpp>
#include <boost/asio/executor_work_guard.hpp>

#include <atomic>
#include <iostream>
#include <list>
#include "io_worker_factory.h"

namespace pt::io
{

struct io_core::pimpl_t
{
    using u_ptr_t = io_core::pimpl_ptr_t;
    using config_t = io_core::config_t;
    using io_context_t = boost::asio::io_context;
    using thread_array_t = std::vector<std::thread>;
    using future_array_t = std::vector<std::future<void>>;
    using promise_array_t = std::list<std::promise<void>>;

    config_t                m_config;
    io_context_t            m_io_context;
    i_io_worker_factory*    m_worker_factory;


    promise_array_t         m_promises;
    //future_array_t          m_futures;
    std::atomic_bool        m_running;



    static u_ptr_t create(const config_t& config
                           , i_io_worker_factory* worker_factory)
    {
        if (worker_factory == nullptr)
        {
             worker_factory = &io_worker_factory::get_instance();
        }
        return std::make_unique<pimpl_t>(config
                                         , worker_factory);
    }

    pimpl_t(const config_t& config
            , i_io_worker_factory* worker_factory)
        : m_config(config)
        , m_io_context(m_config.total_workers())
        , m_worker_factory(worker_factory)
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
                // m_futures.emplace_back(execute_work([&, i]{ worker_proc(i); }));
                m_promises.emplace_back();
                execute_work(i, m_promises.back());

                //m_futures.emplace_back(execute_work(i));
            }
            return true;
        }

        return false;
    }

    void worker_proc(std::size_t worker_id, std::promise<void>& promise)
    {
        boost::asio::executor_work_guard work(m_io_context.get_executor());
        while(is_running())
        {
            m_io_context.run_one();
            // std::cout << worker_id << ": after run" << std::endl;
        }
        promise.set_value();
    }

    bool stop()
    {
        bool flag = true;
        if (m_running.compare_exchange_strong(flag, false))
        {
            m_io_context.stop();
            for (auto& p : m_promises)
            {
                p.get_future().wait();
            }
            m_promises.clear();

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

    std::size_t workers() const
    {
        return m_promises.size();
    }

    bool execute_work(std::size_t worker_id, std::promise<void>& promise)
    {
        auto executor = [worker_id, &promise, this]
        {
            worker_proc(worker_id, promise);
        };

        return m_worker_factory->execute_worker(std::move(executor));

        //return std::async(std::launch::async, executor);
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

io_core::u_ptr_t io_core::create(const config_t &config
                                 , i_io_worker_factory* worker_factory)
{
    return std::make_unique<io_core>(config
                                     , worker_factory);
}

io_core::io_core(const config_t &config
                 , i_io_worker_factory* worker_factory)
    : m_pimpl(pimpl_t::create(config
                              , worker_factory))
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

std::size_t io_core::workers() const
{
    return m_pimpl->workers();
}

template<>
boost::asio::io_context &io_core::get() const
{
    return m_pimpl->m_io_context;
}

}
