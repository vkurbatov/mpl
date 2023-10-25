#include "task_manager_impl.h"

#include <condition_variable>
#include <thread>
#include <shared_mutex>

#include "tools/utils/sync_base.h"
#include <list>
// #include <map>
#include <future>
#include <queue>

#include <iostream>

namespace mpl
{

class task_manager_impl final: public i_task_manager
{
    using mutex_t = pt::utils::shared_spin_lock;
    using config_t = task_manager_factory::config_t;

    struct task_queue_t
    {
        mutable mutex_t                 m_safe_mutex;

        struct task_impl : public i_task
        {
            using s_ptr_t = std::shared_ptr<task_impl>;
            using queue_t = std::queue<s_ptr_t>;
            using promise_t = std::promise<void>;
            using future_t = std::future<void>;

            task_queue_t&               m_owner;
            task_id_t                   m_task_id;
            task_handler_t              m_handler;

            std::atomic<task_state_t>   m_state;

            promise_t                   m_promise;
            std::atomic_bool            m_completed;

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
                , m_completed(false)
            {

            }

            ~task_impl()
            {
                while (m_state.load(std::memory_order_relaxed) == task_state_t::execute)
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
                    m_handler();
                    complete(task_state_t::completed);
                }
            }

            inline bool complete()
            {
                bool flag = false;
                if (m_completed.compare_exchange_strong(flag
                                                        , true))
                {
                    m_promise.set_value();
                    return true;
                }

                return false;
            }

            inline void complete(task_state_t state)
            {
                if (complete())
                {
                    m_state.store(state, std::memory_order_release);
                }
            }

            inline bool is_processed() const
            {
                return !m_completed.load(std::memory_order_acquire);
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
                future.wait();
                return true;
            }

            void cancel() override
            {
                task_state_t need_state = task_state_t::ready;
                if (m_state.compare_exchange_strong(need_state
                                                    , task_state_t::cancelled))
                {
                    complete(task_state_t::cancelled);
                }
            }

            task_state_t state() const override
            {
                return m_state.load(std::memory_order_acquire);
            }
        };

        task_id_t                   m_task_id;

        task_impl::queue_t          m_tasks;

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

                m_tasks.push(task);
                m_task_id++;
                return task;
            }

            return nullptr;
        }

        std::size_t pending() const
        {
            std::shared_lock lock(m_safe_mutex);
            return m_tasks.size();
        }

        task_impl::s_ptr_t fetch_task()
        {
            std::lock_guard lock(m_safe_mutex);

            if (!m_tasks.empty())
            {
                auto task = std::move(m_tasks.front());
                m_tasks.pop();
                return task;
            }
            return nullptr;
        }

        void reset()
        {
            std::lock_guard lock(m_safe_mutex);
            while(!m_tasks.empty())
            {
                m_tasks.front()->cancel();
                m_tasks.pop();
            }
        }
    };

    struct worker_t
    {
        using list_t = std::list<worker_t>;

        task_manager_impl&      m_manager;
        std::size_t             m_worker_id;

        std::thread             m_thread;


        worker_t(task_manager_impl& manager
                 , std::size_t worker_id)
            : m_manager(manager)
            , m_worker_id(worker_id)
            , m_thread([&] { worker_proc(); })
        {

        }

        ~worker_t()
        {
            if (m_thread.joinable())
            {
                m_thread.join();
            }
        }

        void worker_proc()
        {
            std::mutex                      wait_mutex;
            std::unique_lock<std::mutex>    wait_lock(wait_mutex);

            while(m_manager.is_running())
            {
                if (auto task = m_manager.m_task_queue.fetch_task())
                {
                    task->execute();
                }
                else
                {
                    m_manager.wait(wait_lock);
                    // std::clog << "Wakeup #" << m_worker_id << " worker id" << std::endl;
                }
            }
        }
    };

    // mutable std::mutex                  m_sync_mutex;

    config_t                            m_config;

    task_queue_t                        m_task_queue;
    worker_t::list_t                    m_workers;

    std::condition_variable             m_signal;
    std::atomic_bool                    m_running;

public:

    static u_ptr_t create(const config_t &config)
    {
        return std::make_unique<task_manager_impl>(config);
    }

    task_manager_impl(const config_t &config)
        : m_config(config)
        , m_running(false)
    {
        if (m_config.auto_start)
        {
            task_manager_impl::start();
        }
    }

    ~task_manager_impl()
    {
        task_manager_impl::stop();

    }

    inline void wait(std::unique_lock<std::mutex>& lock)
    {
        auto predicate = [&]
        {
            return m_task_queue.pending() > 0 || !is_running();
        };
        m_signal.wait(lock, predicate);
    }

    inline void notify(bool all = false)
    {
        all ? m_signal.notify_all() : m_signal.notify_one();
    }

    inline bool is_running() const
    {
        return m_running.load(std::memory_order_acquire);
    }

    // i_task_manager interface
public:
    i_task::s_ptr_t add_task(const task_handler_t &task_handler) override
    {
        if (is_running())
        {
            if (auto task = m_task_queue.add_task(task_handler))
            {
                notify();
                return task;
            }
        }

        return nullptr;
    }


    void reset() override
    {
        m_task_queue.reset();
    }

    std::size_t pending_tasks() const override
    {
        return m_task_queue.pending();
    }

    std::size_t active_workers() const override
    {
        return m_workers.size();
    }

    bool start() override
    {
        bool flag = false;
        if (m_running.compare_exchange_strong(flag, true))
        {
            auto worker_count = m_config.max_workers;
            if (worker_count == 0)
            {
                worker_count = std::thread::hardware_concurrency();

            }
            for (std::size_t id = 0; id < worker_count; id++)
            {
                m_workers.emplace_back(*this
                                       , id);
            }

            return true;
        }

        return false;
    }

    bool stop() override
    {
        bool flag = true;
        if (m_running.compare_exchange_strong(flag, false))
        {
            m_running.store(false, std::memory_order_release);
            notify(true);
            m_workers.clear();
        }

        return false;
    }

    bool is_started() const override
    {
        return is_running();
    }
};

task_manager_factory &task_manager_factory::get_instance()
{
    static task_manager_factory single_factory;
    return single_factory;
}

i_task_manager &task_manager_factory::single_manager()
{
    static task_manager_impl single_manager({true});
    return single_manager;
}

i_task_manager::u_ptr_t task_manager_factory::create_manager(const config_t &config)
{
    return task_manager_impl::create(config);
}

}
