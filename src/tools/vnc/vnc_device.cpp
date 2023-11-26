#include "vnc_device.h"

extern "C"
{
#include <rfb/rfbclient.h>
#include <rfb/rfbproto.h>
};

#include <sys/ioctl.h>

#include <cstring>
#include <mutex>
#include <thread>
#include <atomic>
#include <iostream>
#include <cstdarg>

#define RFB_PIXEL_FORMAT_DEFAULT 8, 3, 4

#define RFB_PIXEL_FORMAT_32 8, 3, 4
#define RFB_PIXEL_FORMAT_24 8, 3, 3
#define RFB_PIXEL_FORMAT_16 6, 16, 16
#define RFB_PIXEL_FORMAT_15 4, 15, 16
#define RFB_PIXEL_FORMAT_8 3, 8, 8

namespace pt::vnc
{

std::uint32_t owner_tag = 0xadaf;
const std::size_t max_frame_queue_size = 100;
const std::size_t max_key_state_queue_size = 10;


typedef std::pair<std::uint32_t, bool> key_state_t;
typedef std::queue<key_state_t> key_state_queue_t;

static bool connect(rfbClient& rfb_client)
{
    if (!rfb_client.listenSpecified)
    {
        if (!rfb_client.serverHost)
        {
            return false;
        }

        if (rfb_client.destHost != nullptr)
        {
            if (!ConnectToRFBRepeater(&rfb_client
                                      , rfb_client.serverHost
                                      , rfb_client.serverPort
                                      , rfb_client.destHost
                                      , rfb_client.destPort))
            {
                return false;
            }
        }
        else
        {

            if (!ConnectToRFBServer(&rfb_client
                                    , rfb_client.serverHost
                                    , rfb_client.serverPort))
            {
                return false;
            }
        }
    }

    if (InitialiseRFBConnection(&rfb_client))
    {
        rfb_client.width = rfb_client.si.framebufferWidth;
        rfb_client.height = rfb_client.si.framebufferHeight;
        rfb_client.MallocFrameBuffer(&rfb_client);

        return true;
    }

    return false;
}

static bool set_scale_setting(rfbClient& rfb_client)
{
    if (rfb_client.updateRect.x < 0)
    {
        rfb_client.updateRect.x = 0;
        rfb_client.updateRect.y = 0;
        rfb_client.updateRect.w = rfb_client.width;
        rfb_client.updateRect.h = rfb_client.height;
    }

    if (rfb_client.appData.scaleSetting > 1)
    {
        if (!SendScaleSetting(&rfb_client
                              , rfb_client.appData.scaleSetting))
        {
            return false;
        }

        if (!SendFramebufferUpdateRequest(&rfb_client,
                                          rfb_client.updateRect.x / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.y / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.w / rfb_client.appData.scaleSetting,
                                          rfb_client.updateRect.h / rfb_client.appData.scaleSetting,
                                          false))
        {
            return false;
        }
    }
    else
    {
        if (!SendFramebufferUpdateRequest(&rfb_client,
                                          rfb_client.updateRect.x, rfb_client.updateRect.y,
                                          rfb_client.updateRect.w, rfb_client.updateRect.h,
                                          false))
        {
            return false;
        }
    }

    return true;
}

struct vnc_client_t
{
    using u_ptr_t = std::unique_ptr<vnc_client_t>;

    rfbClient*              native_client;
    vnc_config_t            config;
    std::size_t             frame_counter;

    bool                    is_init;

    frame_t                 frame;

    static u_ptr_t create(const vnc_config_t& config)
    {
        if (config.is_valid())
        {
            if (auto client = std::make_unique<vnc_client_t>(config))
            {
                if (client->is_init)
                {
                    return client;
                }
            }
        }

        return nullptr;
    }

    vnc_client_t(const vnc_config_t& config)
        : native_client(nullptr)
        , config(config)
        , frame_counter(0)
        , is_init(false)
    {
        //rfbClientLog = rfbClientErr = rfb_logging;
        is_init = init(this->config);
    }

    ~vnc_client_t()
    {
        if (native_client != nullptr)
        {
            if (native_client->sock >= 0)
            {
                ::close(native_client->sock);
            }

            native_client->frameBuffer = nullptr;
            native_client->serverHost = nullptr;
        }

        rfbClientCleanup(native_client);
        native_client = nullptr;
    }

    bool init(const vnc_config_t& config)
    {
        native_client = rfbGetClient(RFB_PIXEL_FORMAT_32);

        if (native_client != nullptr)
        {
            native_client->MallocFrameBuffer = [](rfbClient* client) -> rfbBool
            {
                if (auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                        , &owner_tag)))
                {
                    vnc_client->frame.frame_size.width = client->width;
                    vnc_client->frame.frame_size.height = client->height;
                    vnc_client->frame.bpp = client->format.bitsPerPixel;
                    vnc_client->frame.realloc();
                    client->frameBuffer = vnc_client->frame.frame_data.data();

                    client->updateRect.x = 0;
                    client->updateRect.y = 0;
                    client->updateRect.w = client->width;
                    client->updateRect.h = client->height;

                    return SetFormatAndEncodings(client);
                }

                return false;
            };

            native_client->GotFrameBufferUpdate = [](rfbClient* client
                                                  , int x
                                                  , int y
                                                  , int width
                                                  , int height)
            {
                if (auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                        , &owner_tag)))
                {
                    vnc_client->frame_counter++;
                }
            };

            native_client->GetPassword = [](rfbClient* client) -> char*
            {
                if (auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                        , &owner_tag)))
                {
                    return strdup(vnc_client->config.password.c_str());
                }

                return nullptr;
            };

            native_client->canHandleNewFBSize = true;
            native_client->frameBuffer = nullptr;
            native_client->programName = nullptr;
            native_client->serverHost = const_cast<char*>(config.host.c_str());
            native_client->serverPort = config.port;

            rfbClientSetClientData(native_client
                                   , &owner_tag
                                   , this);

            return connect(*native_client)
                    && set_scale_setting(*native_client);

        }

        return false;
    }

    bool send_key_state(const key_state_t& key_state)
    {
        if (is_init)
        {
            return SendKeyEvent(native_client
                                , key_state.first
                                , rfbBool(key_state.second));
        }

        return false;
    }


    io_result_t fetch_frame(frame_t& frame
                            , std::uint32_t timeout = 0)
    {
        if (is_init)
        {
            auto result = WaitForMessage(native_client, timeout * 1000);

            if(result > 0
                    && HandleRFBServerMessage(native_client))
            {
                frame = this->frame;

                return io_result_t::complete;
            }

            return result == 0
                    ? io_result_t::timeout
                    : io_result_t::error;
        }
        return io_result_t::not_ready;
    }
};


struct vnc_device_context_t
{
    vnc_config_t                    m_config;

    vnc_client_t::u_ptr_t           m_client;

    frame_queue_t                   m_frame_queue;

    vnc_device_context_t(const vnc_config_t& config)
        : m_config(config)
    {

    }

    ~vnc_device_context_t()
    {
        close();
    }

    bool open()
    {
        if (!is_open())
        {
            if (auto client = vnc_client_t::create(m_config))
            {
                m_client = std::move(client);
                return true;
            }
        }

        return false;
    }

    bool close()
    {
        if (is_open())
        {
            m_client.reset();
            return true;
        }

        return false;
    }

    bool is_open() const
    {
        return m_client != nullptr;
    }

    bool set_config(const vnc_config_t& config)
    {
        if (config.is_valid()
                && !is_open())
        {
            m_config = config;
        }

        return false;
    }


    bool send_key_event(uint32_t virtual_key
                        , bool is_down)
    {
        if (auto client = m_client.get())
        {
            return client->send_key_state({virtual_key, is_down});
        }

        return false;
    }

    io_result_t fetch_frame(frame_t& frame
                     , std::uint32_t timeout)
    {
        if (auto client = m_client.get())
        {
            return client->fetch_frame(frame
                                      , timeout);
        }

        return io_result_t::not_ready;
    }
};

vnc_device::vnc_device(const vnc_config_t& config)
    : m_vnc_device_context(new vnc_device_context_t(config))
{

}

vnc_device::~vnc_device()
{

}

bool vnc_device::open()
{
    return m_vnc_device_context->open();
}


bool vnc_device::close()
{
    return m_vnc_device_context->close();
}

bool vnc_device::is_opened() const
{
    return m_vnc_device_context->is_open();
}

const vnc_config_t &vnc_device::config() const
{
    return m_vnc_device_context->m_config;
}

bool vnc_device::set_config(const vnc_config_t &config)
{
    return m_vnc_device_context->set_config(config);
}

bool vnc_device::send_key_event(uint32_t virtual_key
                                , bool is_down)
{
    return m_vnc_device_context->send_key_event(virtual_key
                                                , is_down);
}

io_result_t vnc_device::fetch_frame(frame_t &frame
                                    , std::uint32_t timeout)
{
    return m_vnc_device_context->fetch_frame(frame
                                             , timeout);
}

}
