#ifndef SERIAL_BASE_H
#define SERIAL_BASE_H

#include <string>
#include <vector>

namespace serial
{

typedef std::uint32_t timeout_t;
typedef std::vector<std::uint8_t> frame_data_t;
typedef std::vector<std::string> serial_list_t;

const timeout_t no_timeout = 0;
const timeout_t infinite_timeout = 0xffffffff;

enum class baud_rate_t : std::uint32_t
{
    br_0        = 0000000,
    br_50       = 0000001,
    br_75       = 0000002,
    br_110      = 0000003,
    br_134      = 0000004,
    br_150      = 0000005,
    br_200      = 0000006,
    br_300      = 0000007,
    br_600      = 0000010,
    br_1200     = 0000011,
    br_1800     = 0000012,
    br_2400     = 0000013,
    br_4800     = 0000014,
    br_9600     = 0000015,
    br_19200	= 0000016,
    br_38400	= 0000017,
    br_undefine = 0010000,
    br_57600    = 0010001,
    br_115200   = 0010002,
    br_230400   = 0010003,
    br_460800   = 0010004,
    br_500000   = 0010005,
    br_576000   = 0010006,
    br_921600   = 0010007,
    br_1000000  = 0010010,
    br_1152000  = 0010011,
    br_1500000  = 0010012,
    br_2000000  = 0010013,
    br_2500000  = 0010014,
    br_3000000  = 0010015,
    br_3500000	= 0010016,
    br_4000000	= 0010017
};

enum class parity_t : std::uint32_t
{
    p_none      = 0000000,
    p_even      = 0000400,
    p_odd       = 0001400
};

enum class stop_bits_t : std::uint32_t
{
    sb_1        = 0000000,
    sb_2        = 0000100
};

enum class data_bits_t : std::uint32_t
{
    db_5        = 0000000,
    db_6        = 0000020,
    db_7        = 0000040,
    db_8        = 0000060
};

enum class flow_control_t : std::uint32_t
{
    fc_no_flow  = 000000000000,
    fc_rts_cts  = 020000000000
};

enum class wait_status_t
{
    success,
    timeout,
    error
};

struct control_config_t
{
    baud_rate_t     baud_rate;
    parity_t        parity;
    stop_bits_t     stop_bits;
    data_bits_t     data_bits;
    flow_control_t  flow_control;

    template<typename T>
    static std::uint32_t& set_control_param(std::uint32_t& c_flags, T param);

    template<typename T>
    static T get_control_param(const std::uint32_t& c_flags);

    control_config_t(baud_rate_t baud_rate = baud_rate_t::br_9600
                     , parity_t parity = parity_t::p_none
                     , stop_bits_t stop_bits = stop_bits_t::sb_1
                     , data_bits_t data_bits = data_bits_t::db_8
                     , flow_control_t flow_control = flow_control_t::fc_no_flow);

    control_config_t(std::uint32_t c_flags);

    std::uint32_t get_flags() const;
    std::uint32_t& update_flags(std::uint32_t& c_flags) const;
    void set_flags(std::uint32_t c_flags);
};

}

#endif // SERIAL_BASE_H
