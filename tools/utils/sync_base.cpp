#include "sync_base.h"

#include <thread>
#include <cassert>

namespace pt::utils
{

constexpr std::size_t idle_length = 100000;

spin_lock::spin_lock()
    : m_spin(ATOMIC_FLAG_INIT)
    , m_count(0)
{
    m_spin.clear();
}


spin_lock::~spin_lock()
{
    assert(!m_spin.test_and_set(std::memory_order_relaxed));
}

void spin_lock::lock()
{
    size_t idl = 0;
    while (m_spin.test_and_set(std::memory_order_acquire))
    {
        if (++idl % idle_length == 0)
        {
            std::this_thread::yield();
        }
    }
}


bool spin_lock::try_lock()
{
    return !m_spin.test_and_set(std::memory_order_acquire);
}

void spin_lock::unlock()
{
    m_spin.clear(std::memory_order_release);
}


shared_spin_lock::shared_spin_lock()
    : m_count(0)
{

}

void shared_spin_lock::lock()
{
    spin_lock::lock();
    while(m_count.load(std::memory_order_acquire) > 0)
    {
        std::this_thread::yield();
    }
}

void shared_spin_lock::lock_shared()
{
    spin_lock::lock();
    m_count.fetch_add(1, std::memory_order_acquire);
    spin_lock::unlock();
}

bool shared_spin_lock::try_lock_shared()
{
    if (spin_lock::try_lock())
    {
        m_count.fetch_add(1, std::memory_order_acquire);
        spin_lock::unlock();
        return true;
    }

    return false;
}

void shared_spin_lock::unlock_shared()
{
    m_count.fetch_sub(1, std::memory_order_release);
}

void fake_lock::lock()
{

}

bool fake_lock::try_lock()
{
    return true;
}

void fake_lock::unlock()
{

}

}
