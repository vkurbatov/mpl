#ifndef MPL_TASK_MANAGER_IMPL_H
#define MPL_TASK_MANAGER_IMPL_H

#include "core/time_types.h"
#include "core/i_task_manager.h"

namespace mpl
{

class task_manager_impl : public i_task_manager
{
public:
    struct config_t
    {
        bool            auto_start = false;
        std::uint32_t   max_workers = 0; // auto
        // std::uint32_t   max_queued_workers = 0; // infinite
    };
private:
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t m_pimpl;

public:

    using u_ptr_t = std::unique_ptr<task_manager_impl>;
    using s_ptr_t = std::shared_ptr<task_manager_impl>;

    static task_manager_impl& get_instance();

    static u_ptr_t create(const config_t& config);

    task_manager_impl(const config_t& config);
    ~task_manager_impl();

    template<typename T>
    T& get();

    // i_task_manager interface
public:
    i_task::s_ptr_t add_task(const task_handler_t &task_handler) override;
    void reset() override;
    std::size_t pending_tasks() const override;
    std::size_t active_workers() const override;
    bool start() override;
    bool stop() override;
    bool is_started() const override;
};

}

#endif // MPL_TASK_MANAGER_IMPL_H
