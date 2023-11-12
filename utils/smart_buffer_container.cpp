#include "smart_buffer_container.h"

namespace mpl::utils
{

smart_buffer_container::smart_buffer_container(const smart_buffer &buffer)
    : m_buffer(buffer)
{

}

smart_buffer_container::smart_buffer_container(smart_buffer &&buffer)
    : m_buffer(std::move(buffer))
{

}

smart_buffer &smart_buffer_container::get_buffer()
{
    return m_buffer;
}

const smart_buffer &smart_buffer_container::get_buffer() const
{
    return m_buffer;
}

void smart_buffer_container::set_buffer(const smart_buffer &buffer)
{
    m_buffer = buffer;
}

void smart_buffer_container::set_buffer(smart_buffer &&buffer)
{
    m_buffer = std::move(buffer);
}

smart_buffer smart_buffer_container::release_buffer()
{
    return std::move(m_buffer);
}

}
