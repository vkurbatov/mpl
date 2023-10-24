#include "task_manager_impl.h"

#include "core/thread_info.h"

#include <condition_variable>
#include <thread>
#include <shared_mutex>

#include "tools/utils/sync_base.h"
#include "tools/io/io_core.h"

#include <list>
#include <map>
#include <future>

#include <iostream>

namespace mpl
{

class task_manager_impl::pimpl_t
{
    using u_ptr_t = std::unique_ptr<pimpl_t>;
    using mutex_t = pt::utils::shared_spin_lock;
    using config_t = task_manager_impl::config_t;
    using task_handler_t = i_task_manager::task_handler_t;

    struct task_queue_t
    {
        mutable mutex_t                 m_safe_mutex;

        struct task_impl : public i_task
        {
            using s_ptr_t = std::shared_ptr<task_impl>;
            using map_t = std::map<task_id_t, task_impl::s_ptr_t>;
            using promise_t = std::promise<void>;
            using future_t = std::future<void>;

            task_queue_t&               m_owner;
            task_id_t                   m_task_id;
            task_handler_t              m_handler;

            std::atomic<task_state_t>   m_state;

            promise_t                   m_promise;

            static s_ptr_t create(task_queue_t& owner
                                  , task_id_t task_id
                                  , const task_handler_t& handler)
            {
                if (handler != nullptr)
                {
                    return std::make_shared<task_impl>(owner
                                                       , task_id
                                                       , handler);
                }

                return nullptr;
            }


            task_impl(task_queue_t& owner
                      , task_id_t task_id
                      , const task_handler_t& handler)
                : m_owner(owner)
                , m_task_id(task_id)
                , m_handler(handler)
                , m_state(task_state_t::ready)
            {

            }

            ~task_impl()
            {
                while(m_state.load(std::memory_order_acquire) == task_state_t::execute)
                {
                    std::this_thread::yield();
                }
            }

            void execute()
            {
                task_state_t need_state = task_state_t::ready;
                if (m_state.compare_exchange_strong(need_state
                                                    , task_state_t::execute))
                {

                    /*std::cout << "Exec task #" << m_task_id
                              << ", from thread " << thread_info_t::current().name
                              << std::endl;*/
                    m_handler();
                    completed();
                }
            }

            inline void completed(task_state_t state = task_state_t::completed)
            {
                m_state.store(state, std::memory_order_release);
                m_promise.set_value();
            }

            inline bool is_process() const
            {
                switch(m_state.load(std::memory_order_acquire))
                {
                    case task_state_t::ready:
                    case task_state_t::execute:
                        return true;
                    break;
                    default:;
                }

                return false;
            }
            // i_task interface

        public:
            task_id_t task_id() const override
            {
                return m_task_id;
            }

            bool wait() override
            {
                auto future = m_promise.get_future();
                if (is_process())
                {
                    future.wait();
                    return true;
                }

                return false;
            }

            void cancel() override
            {
                // completed(task_state_t::cancelled);
                m_owner.remove_task(m_task_id);
            }

            task_state_t state() const override
            {
                return m_state;
            }
        };

        task_id_t                   m_task_id;

        task_impl::map_t            m_tasks;

        task_queue_t()
            : m_task_id(0)
        {

        }

        ~task_queue_t()
        {
            reset();
        }

        task_impl::s_ptr_t add_task(const task_handler_t& task_handler)
        {
            std::lock_guard lock(m_safe_mutex);
            if (auto task = task_impl::create(*this
                                              , m_task_id
                                              , task_handler))
            {

                m_tasks[m_task_id] = task;
                m_task_id++;
                return task;
            }

            return nullptr;
        }

        bool remove_task(task_id_t task_id)
        {
            std::lock_guard lock(m_safe_mutex);
            return m_tasks.erase(task_id) > 0;
        }

        std::size_t pending() const
        {
            std::shared_lock lock(m_safe_mutex);
            return m_tasks.size();
        }

        task_impl::s_ptr_t fetch_task()
        {
            std::lock_guard lock(m_safe_mutex);

            if (auto it = m_tasks.begin(); it != m_tasks.end())
            {
                auto task = std::move(it->second);
                m_tasks.erase(it);
                return task;
            }
            return nullptr;
        }

        void reset()
        {
            std::lock_guard lock(m_safe_mutex);
            m_tasks.clear();
        }
    };

    // mutable std::mutex                  m_sync_mutex;

    config_t                            m_config;
    pt::io::io_core                     m_io_core;

    task_queue_t                        m_task_queue;
    //worker_t::list_t                    m_workers;


public:

    static u_ptr_t create(const config_t &config)
    {
        return std::make_unique<pimpl_t>(config);
    }

    pimpl_t(const config_t &config)
        : m_config(config)
        , m_io_core(config.max_workers)
    {
        if (m_config.auto_start)
        {
            start();
        }
        /*
        auto worker_count = m_config.max_workers;
        if (worker_count == 0)
        {
            worker_count = std::thread::hardware_concurrency();
        }

        for (std::size_t id = 0; id < worker_count; id++)
        {
            m_workers.emplace_back(*this
                                   , id);
        }*/
    }

    ~pimpl_t()
    {
        stop();
        // m_workers.clear();

    }

    inline pt::io::io_core& io_core()
    {
        return m_io_core;
    }

    inline bool is_running() const
    {
        return m_io_core.is_running();
    }

    // i_task_manager interface
public:
    inline i_task::s_ptr_t add_task(const task_handler_t &task_handler)
    {
        if (is_running())
        {
            if (auto task = m_task_queue.add_task(task_handler))
            {
                auto task_handle = [&]
                {
                    if (auto task = m_task_queue.fetch_task())
                    {
                        task->execute();
                    }
                };
                m_io_core.post(task_handle);
                return task;
            }
        }

        return nullptr;
    }


    inline void reset()
    {
        m_task_queue.reset();
    }

    inline std::size_t pending_tasks() const
    {
        return m_task_queue.pending();
    }

    inline std::size_t active_workers() const
    {
        return m_io_core.workers();
    }

    inline bool start()
    {
        return m_io_core.run();
    }

    inline bool stop()
    {
        return m_io_core.stop();
    }

    inline bool is_started() const
    {
        return m_io_core.is_running();
    }
};

task_manager_impl &task_manager_impl::get_instance()
{
    static task_manager_impl single_task_manager({ true } );
    return single_task_manager;
}

task_manager_impl::u_ptr_t task_manager_impl::create(const config_t &config)
{
    return std::make_unique<task_manager_impl>(config);
}

task_manager_impl::task_manager_impl(const config_t &config)
    : m_pimpl(pimpl_t::create(config))
{

}

task_manager_impl::~task_manager_impl()
{

}

i_task::s_ptr_t task_manager_impl::add_task(const task_handler_t &task_handler)
{
    return m_pimpl->add_task(task_handler);
}

void task_manager_impl::reset()
{
    m_pimpl->reset();
}

std::size_t task_manager_impl::pending_tasks() const
{
    return m_pimpl->pending_tasks();
}

std::size_t task_manager_impl::active_workers() const
{
    return m_pimpl->active_workers();
}

bool task_manager_impl::start()
{
    return m_pimpl->start();
}

bool task_manager_impl::stop()
{
    return m_pimpl->stop();
}

bool task_manager_impl::is_started() const
{
    return m_pimpl->is_started();
}

template<>
pt::io::io_core &task_manager_impl::get()
{
    return m_pimpl->io_core();
}

}
