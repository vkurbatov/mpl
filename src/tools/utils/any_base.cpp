#include "any_base.h"

namespace pt::utils
{

any::any()
{

}

const std::type_info& any::type() const
{
    return m_value.type();
}

bool any::operator ==(const any &other) const
{
    return m_comparator
            && m_comparator(m_value
                            , other.m_value);
}

bool any::operator !=(const any &other) const
{
    return !operator == (other);
}

bool any::merge(const any &other)
{
    return m_merger != nullptr
            && m_merger(m_value
                        , other.m_value);
}

bool any::has_value() const
{
    return m_value.has_value();
}

bool any::has_comparable() const
{
    return m_comparator != nullptr;
}

any::operator bool() const
{
    return has_value();
}


}
