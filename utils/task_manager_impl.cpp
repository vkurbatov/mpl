#include "task_manager_impl.h"

#include <condition_variable>
#include <thread>
#include <shared_mutex>

#include "tools/utils/sync_base.h"
#include <list>
#include <map>
#include <future>

#include <iostream>

namespace mpl
{

class task_manager_impl : public i_task_manager
{
    using mutex_t = pt::utils::shared_spin_lock;
    using config_t = task_manager_factory::config_t;

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

            void execute()
            {
                task_state_t need_state = task_state_t::ready;
                if (m_state.compare_exchange_strong(need_state
                                                    , task_state_t::execute))
                {
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
                    m_manager.m_signal.wait(wait_lock);
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
        , m_running(true)
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
    }

    ~task_manager_impl()
    {
        m_running.store(false, std::memory_order_release);
        m_signal.notify_all();
        m_workers.clear();

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
                m_signal.notify_one();
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
};

task_manager_factory &task_manager_factory::get_instance()
{
    static task_manager_factory single_factory;
    return single_factory;
}

i_task_manager &task_manager_factory::single_manager()
{
    static task_manager_impl single_manager({});
    return single_manager;
}

i_task_manager::u_ptr_t task_manager_factory::create_manager(const config_t &config)
{
    return task_manager_impl::create(config);
}

}
