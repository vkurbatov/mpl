#ifndef IO_SERIAL_LINK_CONFIG_H
#define IO_SERIAL_LINK_CONFIG_H

#include "tools/io/io_base.h"
#include "serial_types.h"

namespace io
{

struct serial_link_config_t : public link_config_t
{
    baud_rate_t             baud_rate;
    char_size_t             char_size;
    serial_stop_bits_t      stop_bits;
    serial_parity_t         parity;
    serial_flow_control_t   flow_control;

    serial_link_config_t(std::uint32_t baud_rate = 0
                        , char_size_t char_size = 0
                        , serial_stop_bits_t stop_bits = serial_stop_bits_t::one
                        , serial_parity_t parity = serial_parity_t::none
                        , serial_flow_control_t flow_control = serial_flow_control_t::none);

    bool is_valid() const final override;
};

}

#endif // IO_SERIAL_LINK_CONFIG_H
