#include "option_container.h"

namespace mpl::utils
{

option_container::option_container(const i_option &options)
    : m_options(options)
{

}

option_container::option_container(option_impl &&options)
    : m_options(std::move(options))
{

}

const option_impl &option_container::get_options() const
{
    return m_options;
}

option_impl &option_container::get_options()
{
    return m_options;
}

void option_container::set_options(const i_option &options)
{
    m_options.assign(options);
}

void option_container::set_options(option_impl &&options)
{
    m_options = std::move(options);
}

}
