#include "core_engine_impl.h"
#include "core/core_engine_config.h"

#include "task_manager_impl.h"
#include "timer_manager_impl.h"
#include "buffer_factory_impl.h"

namespace mpl
{

struct core_engine_impl final : public i_core_engine
{
    core_engine_config_t        m_config;

    i_task_manager::u_ptr_t     m_task_manager;
    i_timer_manager::u_ptr_t    m_timer_manager;
    utils::buffer_factory_impl  m_buffer_factory;

    using u_ptr_t = std::unique_ptr<core_engine_impl>;

    static u_ptr_t create(const core_engine_config_t &config)
    {
        return std::make_unique<core_engine_impl>(config);
    }

    core_engine_impl(const core_engine_config_t &config)
        : m_config(config)
        , m_task_manager(task_manager_factory::get_instance().create_manager({false
                                                                             , m_config.worker_count }))
        , m_timer_manager(timer_manager_factory::get_instance().create_timer_manager({}
                                                                                     , *m_task_manager))
    {

    }

    ~core_engine_impl()
    {
        core_engine_impl::stop();
    }

    bool start() override
    {
        return m_task_manager->start()
                && m_timer_manager->start();
    }

    bool stop() override
    {
        return m_task_manager->stop()
                & m_timer_manager->stop();
    }

    bool is_started() const override
    {
        return m_task_manager->is_started()
                && m_timer_manager->is_started();
    }

    // i_core_engine interface
public:
    i_task_manager &task_manager() override
    {
        return *m_task_manager;
    }

    i_timer_manager &timer_manager() override
    {
        return *m_timer_manager;
    }

    i_buffer_factory &buffer_factory() override
    {
        return m_buffer_factory;
    }
};

core_engine_factory &core_engine_factory::get_instance()
{
    static core_engine_factory single_factory;
    return single_factory;
}

i_core_engine::u_ptr_t core_engine_factory::create_engine(const core_engine_config_t &config)
{
    return core_engine_impl::create(config);
}


}
