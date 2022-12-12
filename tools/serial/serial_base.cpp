#include "serial_base.h"

#include <termios.h>

namespace serial
{

const std::uint32_t baud_rate_mask = CBAUD;
const std::uint32_t parity_mask = PARENB | PARODD;
const std::uint32_t stop_bits_mask = CSTOPB;
const std::uint32_t data_bits_mask = CSIZE;
const std::uint32_t flow_control_mask = CRTSCTS;

template<>
uint32_t &control_config_t::set_control_param(uint32_t &c_flags
                                              , baud_rate_t param)
{
    c_flags &= ~baud_rate_mask;
    c_flags |= static_cast<std::uint32_t>(param);
    return c_flags;
}

template<>
baud_rate_t control_config_t::get_control_param(const uint32_t &c_flags)
{
    return static_cast<baud_rate_t>(c_flags & baud_rate_mask);
}

template<>
uint32_t &control_config_t::set_control_param(uint32_t &c_flags
                                              , parity_t param)
{
    c_flags &= ~parity_mask;
    c_flags |= static_cast<std::uint32_t>(param);
    return c_flags;
}

template<>
parity_t control_config_t::get_control_param(const uint32_t &c_flags)
{
    if ((c_flags & PARENB) != 0)
    {
        return static_cast<parity_t>(c_flags & parity_mask);
    }

    return parity_t::p_none;
}


template<>
uint32_t &control_config_t::set_control_param(uint32_t &c_flags
                                              , stop_bits_t param)
{
    c_flags &= ~stop_bits_mask;
    c_flags |= static_cast<std::uint32_t>(param);
    return c_flags;
}

template<>
data_bits_t control_config_t::get_control_param(const uint32_t &c_flags)
{
    return static_cast<data_bits_t>(c_flags & stop_bits_mask);
}

template<>
uint32_t &control_config_t::set_control_param(uint32_t &c_flags
                                              , data_bits_t param)
{
    c_flags &= ~data_bits_mask;
    c_flags |= static_cast<std::uint32_t>(param);
    return c_flags;
}

template<>
stop_bits_t control_config_t::get_control_param(const uint32_t &c_flags)
{
    return static_cast<stop_bits_t>(c_flags & flow_control_mask);
}

template<>
uint32_t &control_config_t::set_control_param(uint32_t &c_flags
                                              , flow_control_t param)
{
    c_flags &= ~flow_control_mask;
    c_flags |= static_cast<std::uint32_t>(param);
    return c_flags;
}

template<>
flow_control_t control_config_t::get_control_param(const uint32_t &c_flags)
{
    return static_cast<flow_control_t>(c_flags & data_bits_mask);
}

control_config_t::control_config_t(baud_rate_t baud_rate
                                   , parity_t parity
                                   , stop_bits_t stop_bits
                                   , data_bits_t data_bits
                                   , flow_control_t flow_control)
    : baud_rate(baud_rate)
    , parity(parity)
    , stop_bits(stop_bits)
    , data_bits(data_bits)
    , flow_control(flow_control)
{

}

control_config_t::control_config_t(uint32_t c_flags)
{
    set_flags(c_flags);
}

uint32_t control_config_t::get_flags() const
{
    std::uint32_t c_flags = 0;

    return update_flags(c_flags);
}

uint32_t& control_config_t::update_flags(uint32_t &c_flags) const
{
    set_control_param(c_flags
                      , baud_rate);

    set_control_param(c_flags
                      , parity);

    set_control_param(c_flags
                      , stop_bits);

    set_control_param(c_flags
                      , data_bits);

    set_control_param(c_flags
                      , flow_control);

    return c_flags;
}

void control_config_t::set_flags(uint32_t c_flags)
{
    baud_rate       = get_control_param<baud_rate_t>(c_flags);
    parity          = get_control_param<parity_t>(c_flags);
    stop_bits       = get_control_param<stop_bits_t>(c_flags);
    data_bits       = get_control_param<data_bits_t>(c_flags);
    flow_control    = get_control_param<flow_control_t>(c_flags);
}

}
