#include "serial_link_config.h"

namespace io
{

serial_link_config_t::serial_link_config_t(baud_rate_t baud_rate
                                            , char_size_t char_size
                                            , serial_stop_bits_t stop_bits
                                            , serial_parity_t parity
                                            , serial_flow_control_t flow_control)
    : link_config_t(link_type_t::serial)
    , baud_rate(baud_rate)
    , char_size(char_size)
    , stop_bits(stop_bits)
    , parity(parity)
    , flow_control(flow_control)

{

}

bool serial_link_config_t::is_valid() const
{
    return type == link_type_t::serial
            && baud_rate > 0
            && char_size >= min_char_size
            && char_size <= max_char_size;
}


}
