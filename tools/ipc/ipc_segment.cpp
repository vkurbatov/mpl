#include "ipc_segment.h"

#include <boost/thread/thread_time.hpp>

#include <atomic>

namespace ipc
{

struct ipc_segment::segment_ctx_t
{
    using allocator_t = ipc_allocator_t<std::uint8_t>;
    using shared_vector_t = std::vector<std::uint8_t, allocator_t>;

    ipc_mutex_t                     m_mutex;
    ipc_cond_t                      m_condition;
    std::atomic_int32_t             m_notify_counter;
    std::atomic_int32_t             m_ref_counter;
    shared_vector_t                 m_data;

    static segment_ctx_t* create(ipc_shmem_manager_t& manager
                                 , const std::string &name
                                 , std::size_t size)
    {
        try
        {
            segment_ctx_t* segment = manager.find<segment_ctx_t>(name.c_str()).first;

            if (segment != nullptr)
            {
                segment->m_ref_counter++;
            }

            if (segment == nullptr && size > 0)
            {
                segment = manager.construct<segment_ctx_t>(name.c_str())(size, manager.get_segment_manager());
            }

            return segment;
        }
        catch(const std::exception& e)
        {

        }

        return nullptr;
    }

    segment_ctx_t(std::size_t size, allocator_t allocator)
        : m_notify_counter(0)
        , m_ref_counter(1)
        , m_data(size
                 , allocator)
    {

    }

    ~segment_ctx_t()
    {

    }

    void* try_map()
    {
        if (m_mutex.try_lock())
        {
            return m_data.data();
        }

        return nullptr;
    }

    void* map()
    {
        m_mutex.lock();
        return m_data.data();
    }

    void* map(timestamp_t timeout_ns)
    {
        if (m_mutex.timed_lock(boost::get_system_time() + boost::posix_time::microseconds(timeout_ns / 1000)))
        {
            return m_data.data();
        }

        return nullptr;
    }

    void unmap()
    {
        m_mutex.unlock();
    }

    std::size_t size() const
    {
        return m_data.size();
    }

    void notify(bool notify_all)
    {
        m_notify_counter.fetch_add(1, std::memory_order_relaxed);
        if (notify_all)
        {
            m_condition.notify_all();
        }
        else
        {
            m_condition.notify_one();
        }
    }

    void wait(ipc_mutex_t& ipc_mutex
              , std::int32_t& trigger)
    {
        auto ncounter = counter();
        boost::interprocess::scoped_lock<ipc_mutex_t> lock(ipc_mutex);
        m_condition.wait(lock, [&]()
        {
            ncounter = counter();
            return trigger != ncounter;
        });
        trigger = ncounter;
    }

    bool wait(ipc_mutex_t& ipc_mutex
              , std::int32_t& trigger
              , timestamp_t timeout_ns)
    {
        auto ncounter = counter();
        boost::interprocess::scoped_lock<ipc_mutex_t> lock(ipc_mutex);
        auto result = m_condition.timed_wait(lock
                                             , boost::get_system_time() + boost::posix_time::microseconds(timeout_ns / 1000)
                                             , [&]()
        {
            ncounter = counter();
            return trigger == ncounter;
        });
        trigger = ncounter;
        return result;
    }

    std::int32_t counter() const
    {
        return m_notify_counter.load(std::memory_order_acquire);
    }
};

ipc_segment::ipc_segment(const ipc_shmem_manager_ptr_t &manager
                         , const std::string& name
                         , std::size_t size)
    : m_manager(manager)
    , m_segment_ctx(segment_ctx_t::create(*m_manager
                                          , name
                                          , size))
    , m_trigger(0)
{
    if (m_segment_ctx)
    {
        m_trigger = m_segment_ctx->counter();
    }
}

ipc_segment::~ipc_segment()
{
    if (m_segment_ctx != nullptr)
    {
        if (m_segment_ctx->m_ref_counter-- < 2)
        {
            if (m_manager)
            {
                m_manager->destroy_ptr(m_segment_ctx);
            }
        }

        m_segment_ctx = nullptr;
    }
}

void *ipc_segment::map(timestamp_t timeout_ns)
{
    if (m_segment_ctx)
    {
        return m_segment_ctx->map(timeout_ns);
    }

    return nullptr;
}

void *ipc_segment::map()
{
    if (m_segment_ctx)
    {
        return m_segment_ctx->map();
    }

    return nullptr;
}

void ipc_segment::unmap()
{
    if (m_segment_ctx)
    {
        m_segment_ctx->unmap();
    }
}

std::string ipc_segment::name() const
{
    if (m_segment_ctx)
    {
        return boost::interprocess::managed_shared_memory::get_instance_name(m_segment_ctx);
    }

    return {};
}

void ipc_segment::notify(bool all)
{
    if (m_segment_ctx)
    {
        m_segment_ctx->notify(all);
    }
}

void ipc_segment::wait()
{
    if (m_segment_ctx)
    {
        m_segment_ctx->wait(m_sync_mutex
                            , m_trigger);
    }
}

bool ipc_segment::wait(timestamp_t timeout_ns)
{
    if (m_segment_ctx)
    {
        return m_segment_ctx->wait(m_sync_mutex
                                   , m_trigger
                                    , timeout_ns);
    }

    return false;
}

std::size_t ipc_segment::size() const
{
    return m_segment_ctx
            ? m_segment_ctx->size()
            : 0ul;
}

bool ipc_segment::is_valid() const
{
    return m_segment_ctx != nullptr;
}

}
