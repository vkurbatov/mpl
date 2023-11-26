#include "ntp_time.h"

namespace mpl
{

constexpr timestamp_t ntp_offset(durations::days(25567));
constexpr timestamp_t ntp_fractions_in_second(1LL << 32);

ntp_time_t ntp_time_t::from_timestamp(timestamp_t timestamp)
{
    timestamp += ntp_offset;

    auto seconds = durations::to_seconds(timestamp);
    auto fractions = (timestamp % durations::second) * ntp_fractions_in_second / durations::second;

    return ntp_time_t(seconds, fractions);
}

ntp_time_t ntp_time_t::from_ntp_24(std::uint32_t ntp_24)
{
    return ntp_time_t((static_cast<std::uint64_t>(ntp_24) & 0x00ffffff) << 14);
}

ntp_time_t ntp_time_t::from_ntp_32(uint32_t ntp_32)
{
    return ntp_time_t((static_cast<std::uint64_t>(ntp_32)) << 16);
}

uint64_t ntp_time_t::build_ntp_time(uint32_t seconds
                                    , uint32_t fractions)
{
    return (static_cast<std::uint64_t>(seconds) << 32)
            | (static_cast<std::uint64_t>(fractions));
}

ntp_time_t::ntp_time_t(uint64_t time)
    : time(time)
{

}

ntp_time_t::ntp_time_t(uint32_t seconds
                       , uint32_t fractions)
    : time(build_ntp_time(seconds
                          , fractions))
{

}

timestamp_t ntp_time_t::seconds() const
{
    return static_cast<timestamp_t>(time >> 32);
}

timestamp_t ntp_time_t::fractions() const
{
    return static_cast<timestamp_t>(time & 0xffffffff);
}

void ntp_time_t::set(uint32_t seconds
                     , uint32_t fractions)
{
    time = build_ntp_time(seconds
                          , fractions);
}

double ntp_time_t::to_double() const
{
    return static_cast<double>(seconds())
            + static_cast<double>(ntp_fractions_in_second) /  static_cast<double>(fractions());
}

uint32_t ntp_time_t::to_ntp_24() const
{
    return (time >> 14) & 0x00ffffff;
}

uint32_t ntp_time_t::to_ntp_32() const
{
    return (time >> 16) & 0xffffffff;
}

timestamp_t ntp_time_t::to_timestamp() const
{
    return durations::seconds(seconds())
            - ntp_offset
            + 1
            + (fractions() * durations::second) / ntp_fractions_in_second;
}

bool ntp_time_t::is_null() const
{
    return time == 0;
}

bool ntp_time_t::operator ==(const ntp_time_t &other) const
{
    return time == other.time;
}

bool ntp_time_t::operator !=(const ntp_time_t &other) const
{
    return ! operator == (other);
}

}
