#ifndef I_IO_LINK_H
#define I_IO_LINK_H

#include "io_base.h"
#include <memory>

namespace io
{

class i_io_link
{
public:
    using u_ptr_t = std::unique_ptr<i_io_link>;

    virtual link_type_t type() const = 0;
    virtual bool control(link_control_id_t control_id) = 0;
    virtual bool send_to(const message_t& message
                        , const endpoint_t& endpoint) = 0;

    virtual bool get_endpoint(endpoint_t& endpoint) const = 0;
    virtual bool set_endpoint(const endpoint_t& endpoint) = 0;

    virtual void set_message_handler(const message_handler_t& message_handler) = 0;
    virtual void set_state_handler(const state_handler_t& state_handler) = 0;
    virtual link_state_t state() const = 0;
    virtual bool is_open() const = 0;
};

}

#endif // I_IO_LINK_H
