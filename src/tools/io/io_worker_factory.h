#ifndef IO_WORKER_FACTORY_H
#define IO_WORKER_FACTORY_H

#include  "i_io_worker_factory.h"

namespace pt::io
{

class io_worker_factory : public i_io_worker_factory
{
public:
    using worker_proc_t = std::function<void()>;

    static io_worker_factory& get_instance();

    virtual future_t execute_worker(worker_proc_t&& worker_proc);

};

}

#endif // IO_WORKER_FACTORY_H
