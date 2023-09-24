#ifndef VISCA_CONTROL_H
#define VISCA_CONTROL_H

#include "visca_base.h"

#include <memory>
#include <string>

namespace visca
{


class i_visca_channel;
class visca_control
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t m_pimpl;

public:
    visca_control(const visca_config_t& visca_config = visca_config_t()
                  , i_visca_channel* channel = nullptr);
    ~visca_control();

    void set_channel(i_visca_channel* channel);

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

    bool set_zoom(std::int16_t zoom);
    bool get_zoom(std::int16_t& zoom);

    bool get_ptz(double& pan, double& tilt, double& zoom);
    bool set_ptz(double pan, double tilt, double zoom);

    bool zoom_stop();

};

}

#endif // VISCA_CONTROL_H
