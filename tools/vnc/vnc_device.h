#ifndef VNC_DEVICE_H
#define VNC_DEVICE_H

#include "vnc_base.h"

#include <string>
#include <memory>

namespace vnc
{

struct vnc_device_context_t;
using vnc_device_context_ptr_t = std::unique_ptr<vnc_device_context_t>;

class vnc_device
{
    vnc_device_context_ptr_t      m_vnc_device_context;
public:

    vnc_device(const vnc_config_t& config = vnc_config_t());

    ~vnc_device();

    bool open();
    bool close();
    bool is_opened() const;

    const vnc_config_t& config() const;
    bool set_config(const vnc_config_t& config);

    bool send_key_event(std::uint32_t virtual_key
                        , bool is_down);

    io_result_t fetch_frame(frame_t& frame
                            , std::uint32_t timeout = 0);

};

}

#endif // VNC_DEVICE_H
