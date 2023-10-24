#ifndef MPL_THREAD_INFO_H
#define MPL_THREAD_INFO_H

#include <string>
#include <cstdint>

namespace mpl
{

struct thread_info_t
{
    using thread_id_t = std::size_t;

    const thread_id_t id;
    std::string name;

    static thread_info_t& current();

    static void set_thread_name(const std::string_view& name);

private:
    thread_info_t(thread_id_t id);
};

}

#endif // MPL_THREAD_INFO_H
