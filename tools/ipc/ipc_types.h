#ifndef IPC_TYPES_H
#define IPC_TYPES_H

#include <boost/interprocess/sync/interprocess_sharable_mutex.hpp>
#include <boost/interprocess/sync/interprocess_mutex.hpp>
#include <boost/interprocess/sync/interprocess_condition.hpp>
#include <boost/interprocess/managed_shared_memory.hpp>

#include <shared_mutex>

namespace ipc
{

using ipc_shmem_manager_t = boost::interprocess::managed_shared_memory;
using ipc_shared_mutex_t = boost::interprocess::interprocess_sharable_mutex;
using ipc_mutex_t = boost::interprocess::interprocess_mutex;
using ipc_cond_t = boost::interprocess::interprocess_condition;
using timestamp_t = std::uint64_t;

template<typename T = std::uint8_t>
using ipc_allocator_t = boost::interprocess::allocator<T, ipc_shmem_manager_t::segment_manager>;

}

#endif // IPC_TYPES_H
