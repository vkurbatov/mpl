#include "type_info.h"

#include <atomic>

namespace mpl
{

type_info_t::type_info_t(type_id_t type_id
                         , const std::type_info &info)
    : type_id(type_id)
    , info(info)
{

}

type_info_t::type_id_t type_info_t::generate_type_id()
{
    static std::atomic<type_info_t::type_id_t> type_id(0);
    return type_id.fetch_add(1);
}

}
