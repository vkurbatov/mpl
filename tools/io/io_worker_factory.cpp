#include "io_worker_factory.h"

namespace pt::io
{

io_worker_factory &io_worker_factory::get_instance()
{
    static io_worker_factory single_factory;
    return single_factory;
}

i_io_worker_factory::future_t io_worker_factory::execute_worker(worker_proc_t&& worker_proc)
{
    return std::async(std::launch::async, worker_proc);
}

}
