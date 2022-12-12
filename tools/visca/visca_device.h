#ifndef VISCA_DEVICE_H
#define VISCA_DEVICE_H

#include "visca_base.h"

#include <string>
#include <memory>

namespace visca
{

struct visca_device_context_t;
struct visca_device_context_deleter_t { void operator()(visca_device_context_t* visca_device_context_ptr); };

typedef std::unique_ptr<visca_device_context_t, visca_device_context_deleter_t>  visca_device_context_ptr_t;

class visca_device
{
    visca_device_context_ptr_t      m_visca_device_context;
public:

    visca_device(const visca_config_t& visca_config = visca_config_t());

    bool open(const std::string& uri);
    bool close();
    bool is_opened() const;
    bool is_established() const;

    const visca_config_t& config() const;
    bool set_config(const visca_config_t& visca_config);

    bool get_id(std::uint16_t& id);

    bool set_pan_tilt(std::int16_t pan
                      , std::int16_t tilt);

    bool set_pan(std::int16_t pan);

    bool set_tilt(std::int16_t tilt);

    bool get_pan_tilt(std::int16_t& pan
                      , std::int16_t& tilt);

    bool get_pan(std::int16_t& pan);
    bool get_tilt(std::int16_t& tilt);

    bool pan_tilt_home();
    bool pan_tilt_reset();

    bool pan_tilt_stop();

    bool set_zoom(std::uint16_t zoom);
    bool get_zoom(std::uint16_t& zoom);

    bool get_ptz(double& pan, double& tilt, double& zoom);
    bool set_ptz(double pan, double tilt, double zoom);

    bool zoom_stop();

};

}

#endif // VISCA_DEVICE_H
