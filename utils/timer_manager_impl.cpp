#include "timer_manager_impl.h"
#include "task_manager_impl.h"

#include "utils/time_utils.h"

#include <thread>
#include <unordered_map>
#include <map>
#include <mutex>
#include <atomic>
#include <condition_variable>

#include <iostream>

namespace mpl
{

class timer_manager_impl final : public i_timer_manager
{
    using cond_t = std::condition_variable;
    using mutex_t = std::mutex;
    using thread_t = std::thread;
    using config_t = timer_manager_factory::config_t;

    class timer_impl final : public i_timer
    {
        timer_manager_impl&         m_manager;
        timer_id_t                  m_id;
        timer_handler_t             m_handler;

        timestamp_t                 m_target_time;
        std::atomic_bool            m_queued;

        friend class timer_manager_impl;

    public:

        using u_ptr_t = std::unique_ptr<timer_impl>;
        using s_ptr_t = std::shared_ptr<timer_impl>;
        using w_ptr_t = std::weak_ptr<timer_impl>;
        using r_ptr_t = timer_impl*;
        using map_t = std::unordered_map<timer_id_t, r_ptr_t>;
        using time_map_t = std::multimap<timestamp_t, r_ptr_t>;

        static u_ptr_t create(timer_manager_impl& manager
                              , timer_id_t id
                              , const timer_handler_t& handler)
        {

            return std::make_unique<timer_impl>(manager
                                                    , id
                                                    , handler);
        }

        timer_impl(timer_manager_impl& manager
                   , timer_id_t id
                   , const timer_handler_t& handler)
            : m_manager(manager)
            , m_id(id)
            , m_handler(handler)
            , m_target_time(timestamp_null)
            , m_queued(false)
        {
            m_manager.append_timer(this);
        }

        ~timer_impl()
        {
            m_manager.remove_timer(this);
            // std::clog << "timer #" << m_id << ": before destuctor wait loop" << std::endl;
            while(m_queued.load(std::memory_order_relaxed))
            {
                std::this_thread::yield();
            }
        }

        void execute()
        {
            m_target_time = timestamp_null;
            // std::clog << "timer #" << m_id << ": before handler" << std::endl;
            if (m_handler)
            {
                m_handler();
            }
            // std::clog << "timer #" << m_id << ": after handler" << std::endl;
            m_queued.store(false, std::memory_order_release);
        }
        // i_timer interface
    public:
        bool set_handler(const timer_handler_t &handler) override
        {
            if (m_target_time == timestamp_null)
            {
                m_handler = handler;
                return true;
            }

            return false;
        }

        timer_id_t id() const override
        {
            return m_id;
        }

        bool start(timestamp_t timeout) override
        {
            return m_handler != nullptr
                    && m_manager.start_timer(this
                                             , timeout);
        }

        bool stop() override
        {
            if (m_target_time != timestamp_null)
            {
                m_manager.stop_timer(this);
                m_target_time = timestamp_null;
                return true;
            }
            return false;
        }

        bool processed() const override
        {
            return m_target_time != timestamp_null;
        }

        timestamp_t target_time() const override
        {
            return m_target_time;
        }

    };

    mutable mutex_t                 m_safe_mutex;
    cond_t                          m_signal;

    config_t                        m_config;
    i_task_manager&                 m_task_manager;

    timer_impl::map_t               m_timers;
    timer_impl::time_map_t          m_timeouts;

    i_timer::timer_id_t             m_timer_ids;

    thread_t                        m_timer_thread;
    std::atomic_bool                m_started;

    // i_timer_manager interface
public:


    using u_ptr_t = std::unique_ptr<timer_manager_impl>;
    using s_ptr_t = std::shared_ptr<timer_manager_impl>;
    using w_ptr_t = std::weak_ptr<timer_manager_impl>;

    static timestamp_t now()
    {
        return utils::time::get_ticks();
    }

    static u_ptr_t create(const config_t& config
                          , i_task_manager& task_manager)
    {
        return std::make_unique<timer_manager_impl>(config
                                                   , task_manager);
    }

    timer_manager_impl(const config_t& config
                       , i_task_manager& task_manager)
        : m_config(config)
        , m_task_manager(task_manager)
        , m_timer_ids(0)
        , m_started(false)
    {

    }

    ~timer_manager_impl()
    {
        internal_stop();
    }

    bool internal_start()
    {
        bool flag = false;
        if (m_started.compare_exchange_strong(flag, true))
        {
            m_timer_thread = std::move(std::thread([&]{ timer_proc(); }));
            return true;
        }

        return false;
    }

    bool internal_stop()
    {
        bool flag = true;
        if (m_started.compare_exchange_strong(flag, false))
        {
            m_signal.notify_all();
            if (m_timer_thread.joinable())
            {
                m_timer_thread.join();
            }

            return true;
        }

        return false;
    }

    inline bool internal_is_started() const
    {
        return m_started.load(std::memory_order_acquire);
    }

    inline void execute(timer_impl* timer)
    {
        timer->m_queued.store(true, std::memory_order_release);
        // std::clog << "timer #" << timer->id() << ": queued task" << std::endl;
        m_task_manager.add_task([timer] { timer->execute(); });
    }

    void timer_proc()
    {
        std::unique_lock lock(m_safe_mutex);
        while(internal_is_started())
        {
            timestamp_t wait_timeout = timestamp_infinite;
            auto now_timestamp = now();

            if (auto it = m_timeouts.begin(); it != m_timeouts.end())
            {
                if (it->first <= now_timestamp)
                {
                    auto timer = it->second;
                    m_timeouts.erase(it);
                    execute(timer);
                    continue;
                }
                else
                {
                    wait_timeout = it->first - now_timestamp;
                }
            }

            if (wait_timeout == timestamp_infinite)
            {
                m_signal.wait(lock);
            }
            else
            {
                m_signal.wait_for(lock
                                  , std::chrono::nanoseconds(wait_timeout));
            }
        }

        m_timeouts.clear();
    }

    inline bool append_timer(timer_impl::r_ptr_t timer)
    {
        std::lock_guard lock(m_safe_mutex);
        return m_timers.emplace(timer->m_id
                                , timer).second;
    }

    inline bool remove_timer(timer_impl::r_ptr_t timer)
    {
        std::lock_guard lock(m_safe_mutex);
        remove_timeout(timer);
        return m_timers.erase(timer->m_id) > 0;
    }

    inline bool start_timer(timer_impl::r_ptr_t timer
                            , timestamp_t timeout)
    {
        if (internal_is_started())
        {
            std::lock_guard lock(m_safe_mutex);
            remove_timeout(timer);
            if (timeout != timestamp_null)
            {
                auto target_time = now() + timeout;
                auto it = m_timeouts.emplace(target_time
                                                , timer);
                auto first = m_timeouts.begin() == it;
                timer->m_target_time = target_time;
                if (first)
                {
                    m_signal.notify_all();
                }
            }
            else
            {
                execute(timer);
            }
        }
        return true;
    }

    inline bool stop_timer(timer_impl::r_ptr_t timer)
    {
        std::lock_guard lock(m_safe_mutex);
        return remove_timeout(timer);
    }

    bool remove_timeout(timer_impl::r_ptr_t timer)
    {
        if (timer->processed())
        {
            for (auto it = m_timeouts.find(timer->m_target_time)
                 ; it != m_timeouts.end()
                    && it->second->m_target_time == timer->m_target_time
                 ; it++)
            {
                if (it->second == timer)
                {
                    m_timeouts.erase(it);
                    return true;
                }
            }
        }
        return false;
    }

public:

    i_timer::u_ptr_t create_timer(const timer_handler_t &timer_handler) override
    {
        return timer_impl::create(*this
                                  , m_timer_ids++
                                  , timer_handler);
        return nullptr;
    }

    bool start() override
    {
        return internal_start();
    }

    bool stop() override
    {
        return internal_stop();
    }

    bool is_started() const override
    {
        return internal_is_started();
    }
};

timer_manager_factory &timer_manager_factory::get_instance()
{
    static timer_manager_factory single_factory;
    return single_factory;
}

i_timer_manager &timer_manager_factory::single_manager()
{
    static timer_manager_impl single_timer_manager({}
                                                   , task_manager_factory::single_manager());
    return single_timer_manager;
}

i_timer_manager::u_ptr_t timer_manager_factory::create_timer_manager(const config_t &config
                                                                     , i_task_manager &task_manager)
{
    return timer_manager_impl::create(config
                                      , task_manager);
    // return
}



}
