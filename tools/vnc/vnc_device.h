#ifndef VNC_DEVICE_H
#define VNC_DEVICE_H

#include "vnc_base.h"

#include <string>
#include <memory>

namespace vnc
{

struct vnc_device_context_t;
struct vnc_device_context_deleter_t { void operator()(vnc_device_context_t* vnc_device_context_ptr); };

typedef std::unique_ptr<vnc_device_context_t, vnc_device_context_deleter_t>  vnc_device_context_ptr_t;

class vnc_device
{
    vnc_device_context_ptr_t      m_vnc_device_context;
public:

    vnc_device(frame_handler_t frame_handler = nullptr
            , const vnc_config_t& config = vnc_config_t());

    bool open(const vnc_server_config_t& server_config);
    bool open(const std::string& uri);
    bool close();
    bool is_opened() const;
    bool is_established() const;

    const vnc_config_t& config() const;
    bool set_config(const vnc_config_t& config);

    void send_key_event(std::uint32_t virtual_key
                        , bool is_down);

    frame_queue_t fetch_frame_queue();

};

}

#endif // VNC_DEVICE_H
