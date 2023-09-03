#ifndef IO_SERIAL_LINK_H
#define IO_SERIAL_LINK_H

#include "tools/io/io_link_base.h"

namespace io
{

struct serial_link_config_t;
struct serial_endpoint_t;

class serial_link : public io_link
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t     m_pimpl;
    friend struct pimpl_t;
public:

    using u_ptr_t = std::unique_ptr<serial_link>;

    static u_ptr_t create(io_core& core
                          , const serial_link_config_t& config);

    serial_link(io_core& core
                , const serial_link_config_t& config);

    bool set_config(const serial_link_config_t& config);

    const serial_link_config_t& config();
    const serial_endpoint_t& endpoint() const;


    // i_io_link interface
public:
    link_type_t type() const override;
    bool control(link_control_id_t control_id) override;
    bool send_to(const message_t &message, const endpoint_t &endpoint) override;
    bool set_endpoint(const endpoint_t &endpoint) override;
    bool is_open() const override;
};

}

#endif // IO_SERIAL_LINK_H
