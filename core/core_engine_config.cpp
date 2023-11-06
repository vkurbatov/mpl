#include "core_engine_config.h"

namespace mpl
{

core_engine_config_t::core_engine_config_t(std::uint32_t worker_count)
    : worker_count(worker_count)
{

}

bool core_engine_config_t::operator ==(const core_engine_config_t &other) const
{
    return worker_count == other.worker_count;
}

bool core_engine_config_t::operator !=(const core_engine_config_t &other) const
{
    return ! operator == (other);
}


}
