#include "io_worker_factory.h"

namespace pt::io
{

io_worker_factory &io_worker_factory::get_instance()
{
    static io_worker_factory single_factory;
    return single_factory;
}

bool io_worker_factory::execute_worker(const worker_proc_t &worker_proc)
{
    std::async(std::launch::async, worker_proc);
    return true;
}

}
