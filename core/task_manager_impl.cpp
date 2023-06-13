#include "task_manager_impl.h"

#include <condition_variable>
#include <thread>
#include <shared_mutex>

#include "tools/base/sync_base.h"
#include <list>
#include <map>

namespace mpl
{

class task_manager_impl : public i_task_manager
{
    using mutex_t = base::shared_spin_lock;
    using config_t = task_manager_factory::config_t;

    struct task_queue_t
    {
        using task_handler_map_t = std::map<task_id_t, task_handler_t>;
        mutable mutex_t             m_safe_mutex;

        task_id_t                   m_task_id;
        task_handler_map_t          m_tasks;

        task_id_t add_task(const task_handler_t& task_handler)
        {
            if (task_handler != nullptr)
            {
                std::lock_guard lock(m_safe_mutex);
                m_tasks[++m_task_id] = task_handler;
                return m_task_id;
            }

            return task_id_none;
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

        task_handler_t fetch_task()
        {
            std::lock_guard lock(m_safe_mutex);

            if (auto it = m_tasks.begin(); it != m_tasks.end())
            {
                auto handler = std::move(it->second);
                m_tasks.erase(it);
                return handler;
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

            auto predicate = [&]
            {
                return !m_manager.is_running();
            };

            while(m_manager.is_running())
            {
                if (auto handler = m_manager.m_task_queue.fetch_task())
                {
                    handler();
                }
                else
                {
                    m_manager.m_signal.wait(wait_lock, predicate);
                }
            }
        }
    };

    mutable mutex_t                     m_safe_mutex;
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
    task_id_t add_task(const task_handler_t &task_handler) override
    {
        if (is_running())
        {
            auto task_id = m_task_queue.add_task(task_handler);
            if (task_id != task_id_none)
            {
                m_signal.notify_one();
            }
        }

        return task_id_none;
    }

    bool remove_task(task_id_t task_id) override
    {
        return m_task_queue.remove_task(task_id);
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

i_task_manager::u_ptr_t task_manager_factory::create_manager(const config_t &config)
{
    return task_manager_impl::create(config);
}

}
