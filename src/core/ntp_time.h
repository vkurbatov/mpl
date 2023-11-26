#ifndef MPL_NTP_TIME_H
#define MPL_NTP_TIME_H

#include "time_types.h"

namespace mpl
{

struct ntp_time_t
{
    std::uint64_t       time;

    static ntp_time_t from_timestamp(timestamp_t timestamp);
    static ntp_time_t from_ntp_24(std::uint32_t ntp_24);
    static ntp_time_t from_ntp_32(std::uint32_t ntp_32);
    static std::uint64_t build_ntp_time(std::uint32_t seconds
                                        , std::uint32_t fractions);

    ntp_time_t(std::uint64_t time = 0);
    ntp_time_t(std::uint32_t seconds
               , std::uint32_t fractions);

    timestamp_t seconds() const;
    timestamp_t fractions() const;

    void set(std::uint32_t seconds
             , std::uint32_t fractions);

    double to_double() const;
    std::uint32_t to_ntp_24() const;
    std::uint32_t to_ntp_32() const;
    timestamp_t to_timestamp() const;


    bool is_null() const;

    bool operator == (const ntp_time_t& other) const;
    bool operator != (const ntp_time_t& other) const;
};


}

#endif // MPL_NTP_TIME_H
