#include "thread_info.h"


namespace mpl
{

namespace detail
{

thread_info_t::thread_id_t generate_id()
{
    static thread_info_t::thread_id_t single_tread_id = 0;
    return single_tread_id++;
}

}

thread_info_t &thread_info_t::current()
{
    static thread_local thread_info_t local_thread_info(detail::generate_id());

    return local_thread_info;
}

void thread_info_t::set_thread_name(const std::string_view &name)
{
    current().name = name;
}

thread_info_t::thread_info_t(thread_id_t id)
    : id(id)
    , name(std::to_string(id))
{

}



}
