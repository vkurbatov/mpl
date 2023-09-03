#ifndef I_IO_LINK_FACTORY_H
#define I_IO_LINK_FACTORY_H

#include "i_io_link.h"

namespace io
{

struct link_config_t;

class i_io_link_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_io_link_factory>;

    virtual i_io_link::u_ptr_t create_link(const link_config_t& link_config) = 0;
};

}

#endif // I_IO_LINK_FACTORY_H
