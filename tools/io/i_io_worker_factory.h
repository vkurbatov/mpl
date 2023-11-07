#ifndef I_IO_WORKER_FACTORY_H
#define I_IO_WORKER_FACTORY_H

#include <future>
#include <functional>

namespace pt::io
{

class i_io_worker_factory
{
public:

    using future_t = std::future<void>;
    using worker_proc_t = std::function<void()>;
    virtual ~i_io_worker_factory() = default;

    virtual future_t execute_worker(worker_proc_t&& worker_proc) = 0;

};

}

#endif // I_IO_WORKER_FACTORY_H
