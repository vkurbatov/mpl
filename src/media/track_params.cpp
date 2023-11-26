#include "track_params.h"

namespace mpl::media
{

track_params_t::track_params_t(const std::string_view &name
                               , bool enabled)
    : name(name)
    , enabled(enabled)
{

}

bool track_params_t::operator ==(const track_params_t &other) const
{
    return name == other.name
            && enabled == other.enabled;
}

bool track_params_t::operator !=(const track_params_t &other) const
{
    return ! operator == (other);
}


}
