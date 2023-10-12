#ifndef SYNC_BASE_H
#define SYNC_BASE_H

#include <atomic>

namespace portable
{

class spin_lock
{
    std::atomic_flag            m_spin;
    std::atomic<std::size_t>    m_count;
public:
    spin_lock();
    spin_lock(const spin_lock&) = delete;
    ~spin_lock();

    void lock();
    bool try_lock();
    void unlock();
};

class shared_spin_lock : public spin_lock
{
    std::atomic<std::size_t>    m_count;
public:
    shared_spin_lock();
    shared_spin_lock(const spin_lock&) = delete;

    void lock();
    void lock_shared();
    bool try_lock_shared();
    void unlock_shared();
};

class fake_lock
{
    void lock();
    bool try_lock();
    void unlock();
};

}

#endif // SYNC_BASE_H
