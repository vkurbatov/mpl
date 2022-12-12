#ifndef VISCA_PROTO_H
#define VISCA_PROTO_H

#include <cstdint>

namespace visca
{

void set_pan_tilt(void* raw_data
                  , std::int16_t pan
                  , std::int16_t tilt);

void get_pan_tilt(const void* raw_data
                  , std::int16_t& pan
                  , std::int16_t& tilt);

void set_pan_tilt_speed(void* raw_data
                       , std::uint8_t pan_speed
                       , std::uint8_t tilt_speed
                       , std::int16_t pan
                       , std::int16_t tilt);

void set_zoom(void* raw_data
              , std::uint16_t zoom);

void get_zoom(const void* raw_data
              , std::uint16_t& zoom);


void get_version_info(const void* raw_data
                      , std::uint16_t& vendor
                      , std::uint16_t& model
                      , std::uint16_t& rom
                      , std::uint8_t& socket);

void get_id(const void* raw_data
            , std::uint16_t& id);

}

#endif // VISCA_PROTO_H
