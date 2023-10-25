#ifndef IO_CORE_H
#define IO_CORE_H

#include "io_base.h"
#include <memory>
#include <future>

namespace pt::io
{

class i_io_worker_factory;

class io_core
{
public:
    struct config_t
    {
        std::size_t max_workers;
        config_t(std::size_t max_workers = 0);

        std::size_t total_workers() const;
    };

private:
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;

public:

    using future_t = std::future<void>;
    using u_ptr_t = std::unique_ptr<io_core>;

    static io_core& get_instance();
    static u_ptr_t create(const config_t& config = {}
                          , i_io_worker_factory* worker_factory = nullptr);

    io_core(const config_t& config = {}
            , i_io_worker_factory* worker_factory = nullptr);
    ~io_core();
    const config_t& config() const;
    // void set_worker_executor(const executor_handler_t& executor);
    bool run();
    bool stop();
    bool is_running() const;
    void post(const executor_handler_t& executor);
    bool is_valid() const;
    std::size_t workers() const;

    template<typename T>
    T& get() const;
};

}

#endif // IO_CORE_H
