#include "visca_proto.h"

namespace visca
{

static uint32_t to_visca_value(uint32_t normal_value)
{
    return ((normal_value & 0xf000) >> 12)
            | ((normal_value & 0x0f00))
            | ((normal_value & 0x00f0) << 12)
            | ((normal_value & 0x000f) << 24);
}

static uint32_t from_visca_value(uint32_t visca_value)
{
    return ((visca_value & 0x0f000000) >> 24)
            | ((visca_value & 0x000f0000) >> 12)
            | ((visca_value & 0x00000f00))
            | ((visca_value & 0x0000000f) << 12);
}

template<typename T>
static void set_raw_value(void *raw_data
                          , std::int32_t offset
                          , T value)
{
    reinterpret_cast<T&>(*(static_cast<std::uint8_t*>(raw_data) + offset)) = value;
}

template<typename T>
static T get_raw_value(const void *raw_data
                       , std::int32_t offset)
{
    return reinterpret_cast<const T&>(*(static_cast<const std::uint8_t*>(raw_data) + offset));
}

template<typename T>
static void set_visca_value(void *raw_data
                            , std::int32_t offset
                            , std::int16_t value)
{
    set_raw_value<T>(raw_data
                    , offset
                    , to_visca_value(value));
}

template<typename T>
static std::uint32_t get_visca_value(const void *raw_data
                            , std::int32_t offset)
{
    return from_visca_value(get_raw_value<T>(raw_data
                                             , offset));
}

std::uint16_t swap_bytes(std::uint16_t value)
{
    return (value << 8 | value >> 8);
}

void set_pan_tilt(void *raw_data
                  , int16_t pan
                  , int16_t tilt)
{
    set_visca_value<std::uint32_t>(raw_data, 0, pan);
    set_visca_value<std::uint32_t>(raw_data, 4, tilt);
}

void get_pan_tilt(const void *raw_data
                  , int16_t &pan
                  , int16_t &tilt)
{
    pan = get_visca_value<std::uint32_t>(raw_data, 0);
    tilt = get_visca_value<std::uint32_t>(raw_data, 4);
}

void set_pan_tilt_speed(void *raw_data
                        , uint8_t pan_speed
                        , uint8_t tilt_speed
                        , int16_t pan
                        , int16_t tilt)
{
    set_raw_value<std::uint8_t>(raw_data, 0, pan_speed);
    set_raw_value<std::uint8_t>(raw_data, 1, tilt_speed);
    set_visca_value<std::uint32_t>(raw_data, 2, pan);
    set_visca_value<std::uint32_t>(raw_data, 6, tilt);
}

void set_zoom(void *raw_data
              , uint16_t zoom)
{
    set_visca_value<std::uint32_t>(raw_data, 0, zoom);
}

void get_zoom(const void *raw_data
              , uint16_t &zoom)
{
    zoom = get_visca_value<std::uint32_t>(raw_data, 0);
}

void get_version_info(const void *raw_data
                      , std::uint16_t &vendor
                      , std::uint16_t &model
                      , std::uint16_t &rom
                      , uint8_t &socket)
{
    vendor = swap_bytes(get_raw_value<std::uint16_t>(raw_data, 0));
    model = swap_bytes(get_raw_value<std::uint16_t>(raw_data, 2));
    rom = swap_bytes(get_raw_value<std::uint16_t>(raw_data, 4));
    socket = get_raw_value<std::uint8_t>(raw_data, 6);
}

void get_id(const void *raw_data,
            uint16_t &id)
{
    id = get_visca_value<std::uint32_t>(raw_data, 0);
}

}
