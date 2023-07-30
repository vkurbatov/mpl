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

namespace vnc
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
    rfbClient*              native_client;
    vnc_server_config_t     server_config;
    std::size_t             frame_counter;
    std::mutex              mutex;

    bool                    is_init;

    frame_t                 frame;

    vnc_client_t(const vnc_server_config_t& server_config)
        : native_client(nullptr)
        , server_config(server_config)
        , frame_counter(0)
        , is_init(false)
    {
        //rfbClientLog = rfbClientErr = rfb_logging;
        is_init = init(this->server_config);
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

    bool init(const vnc_server_config_t& config)
    {
        native_client = rfbGetClient(RFB_PIXEL_FORMAT_32);

        if (native_client != nullptr)
        {
            native_client->MallocFrameBuffer = [](rfbClient* client) -> rfbBool
            {
                if (auto vnc_client = static_cast<vnc_client_t*>(rfbClientGetClientData(client
                                                                                        , &owner_tag)))
                {
                    std::lock_guard<std::mutex> lg(vnc_client->mutex);

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
                    return strdup(vnc_client->server_config.password.c_str());
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
                std::lock_guard<std::mutex> lg(mutex);
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
    frame_handler_t                 m_frame_handler;
    vnc_config_t                    m_config;

    std::thread                     m_stream_thread;
    mutable std::mutex              m_mutex;

    std::atomic_bool                m_running;
    std::atomic_bool                m_established;

    std::unique_ptr<vnc_client_t>   m_client;
    frame_queue_t                   m_frame_queue;
    key_state_queue_t               m_key_state_queue;
    bool                            m_key_send;
    std::atomic_bool                m_open;

    vnc_device_context_t(frame_handler_t frame_handler
                         , const vnc_config_t& config)
        : m_frame_handler(frame_handler)
        , m_config(config)
        , m_running(false)
        , m_established(false)
        , m_key_send(false)
        , m_open(false)
    {

    }

    ~vnc_device_context_t()
    {
        close();
    }

    bool open(const vnc_server_config_t &server_config)
    {
        bool flag = false;
        if (m_open.compare_exchange_strong(flag
                                           , true))
        {
            m_stream_thread = std::thread(&vnc_device_context_t::stream_proc
                                          , this
                                          , server_config);

            return true;

        }

        return false;
    }

    bool close()
    {
        bool flag = true;
        if (m_open.compare_exchange_strong(flag
                                           , false))
        {
            if (m_stream_thread.joinable())
            {
                m_stream_thread.join();
                return true;
            }
        }

        return false;
    }

    bool is_open() const
    {
        return m_open.load(std::memory_order_acquire);
    }

    bool is_established() const
    {
        return is_open()
                && m_established.load(std::memory_order_acquire);
    }

    void process_frame(frame_t&& frame)
    {
        if (m_frame_handler == nullptr
                || m_frame_handler(std::move(frame)) == false)
        {
            std::lock_guard<std::mutex> lg(m_mutex);
            m_frame_queue.emplace(std::move(frame));

            while (m_frame_queue.size() > max_frame_queue_size)
            {
                m_frame_queue.pop();
            }
        }
    }

    frame_queue_t fetch_frame_queue()
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        return std::move(m_frame_queue);
    }

    void send_key_event(uint32_t virtual_key
                        , bool is_down)
    {
        std::lock_guard<std::mutex> lg(m_mutex);
        m_key_state_queue.emplace(virtual_key
                                  , is_down);

        if (m_key_state_queue.size() > max_key_state_queue_size)
        {
            m_key_state_queue.pop();
        }

        m_key_send = true;
    }

    void stream_proc(const vnc_server_config_t &server_config)
    {
        while (is_open())
        {
            {
                std::unique_ptr<vnc_client_t> vnc_client(new vnc_client_t(server_config));

                if (vnc_client->is_init)
                {
                    m_established.store(true, std::memory_order_release);
                    bool is_complete = false;

                    while (is_open()
                           && is_complete == false)
                    {
                        const auto frame_time = 1000 / m_config.fps;

                        if (m_key_send)
                        {
                            std::lock_guard<std::mutex> lg(m_mutex);
                            while (!m_key_state_queue.empty())
                            {
                                vnc_client->send_key_state(m_key_state_queue.front());
                                m_key_state_queue.pop();
                            }
                            m_key_send = false;
                        }

                        frame_t frame;
                        auto io_result = vnc_client->fetch_frame(frame
                                                                 , frame_time * 2);

                        frame.fps = m_config.fps;

                        switch (io_result)
                        {
                            case io_result_t::complete:
                            {
                                process_frame(std::move(frame));
                            }
                            break;
                            case io_result_t::timeout:
                                // nothing
                            break;
                            default:
                            {
                                is_complete = true;
                            }
                        }

                        if (is_complete == false)
                        {
                            std::this_thread::sleep_for(std::chrono::milliseconds(frame_time));
                        }
                    }
                }
            }

            m_established.store(false, std::memory_order_release);

            if (is_open())
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(500));
            }
        }
    }    
};

vnc_device::vnc_device(frame_handler_t frame_handler
                       , const vnc_config_t& config)
    : m_vnc_device_context(new vnc_device_context_t(frame_handler
                                                    , config))
{

}

vnc_device::~vnc_device()
{

}

bool vnc_device::open(const vnc_server_config_t &server_config)
{
    return m_vnc_device_context->open(server_config);
}

bool vnc_device::open(const std::string &uri)
{
    return m_vnc_device_context->open(vnc_server_config_t::from_uri(uri));
}

bool vnc_device::close()
{
    return m_vnc_device_context->close();
}

bool vnc_device::is_opened() const
{
    return m_vnc_device_context->is_open();
}

bool vnc_device::is_established() const
{
    return m_vnc_device_context->m_established;
}

const vnc_config_t &vnc_device::config() const
{
    return m_vnc_device_context->m_config;
}

bool vnc_device::set_config(const vnc_config_t &config)
{
    m_vnc_device_context->m_config = config;
    return true;
}

void vnc_device::send_key_event(uint32_t virtual_key
                                , bool is_down)
{
    return m_vnc_device_context->send_key_event(virtual_key
                                                , is_down);
}

frame_queue_t vnc_device::fetch_frame_queue()
{
    return std::move(m_vnc_device_context->fetch_frame_queue());
}


}
