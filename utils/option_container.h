#ifndef MPL_UTILS_OPTION_CONTAINER_H
#define MPL_UTILS_OPTION_CONTAINER_H

#include "option_impl.h"

namespace mpl::utils
{

class option_container
{
protected:
    option_impl     m_options;

public:
    option_container(const i_option& options);
    option_container(option_impl&& options = {});

    const option_impl& get_options() const;
    option_impl& get_options();

    void set_options(const i_option& options);
    void set_options(option_impl&& options);
};

}

#endif // MPL_UTILS_OPTION_CONTAINER_H
