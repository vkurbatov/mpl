#include "visca_device.h"

#include "tools/serial/serial_device.h"
#include "visca_proto.h"

#include <thread>
#include <cstring>

#include "tools/base/string_base.h"

#define WBS_MODULE_NAME "visca:device"
#include "tools/base/logger_base.h"


namespace visca
{

const std::size_t any_size = 0;
const std::uint32_t request_delay = 100;

static std::size_t get_command_size(visca_command_t command)
{
    switch(command)
    {
        case visca_command_t::cmd_custom:
            return 0;
        break;
        case visca_command_t::cmd_address_set:
            return 1;
        break;
        case visca_command_t::cmd_cancel:
            return 1;
        break;
        default:
            return 3;
    }
}


template<typename Tin = std::uint32_t, typename Tout = std::uint32_t>
static Tout to_visca_value(Tin normal_value)
{
    auto process_value = static_cast<std::uint32_t>(normal_value);
    process_value = ((process_value & 0xf000) >> 12)
                    | ((process_value & 0x0f00))
                    | ((process_value & 0x00f0) << 12)
                    | ((process_value & 0x000f) << 24);

    return static_cast<Tout>(process_value);
}

template<typename Tin = std::uint32_t, typename Tout = std::uint32_t>
static Tout from_visca_value(Tin visca_value)
{
    auto process_value = static_cast<std::uint32_t>(visca_value);

    process_value = ((process_value & 0x0f000000) >> 24)
                    | ((process_value & 0x000f0000) >> 12)
                    | ((process_value & 0x00000f00))
                    | ((process_value & 0x0000000f) << 12);

    return static_cast<Tout>(process_value);
}


template<typename T>
static packet_data_t& push_arg(packet_data_t& args_buffer
                               , const T& value)
{
    auto offset = args_buffer.size();
    args_buffer.resize(args_buffer.size() + sizeof(T));
    std::memcpy(args_buffer.data() + offset, &value, sizeof(T));

    return args_buffer;
}

template<typename T>
static std::size_t pop_args(packet_data_t& args_buffer
                           , std::size_t index
                           , const T& value)
{
    std::memcpy(&value, args_buffer.data() + index, sizeof(T));
    return index + sizeof(T);
}

static packet_data_t& serealize_args(packet_data_t& args_buffer)
{
    return args_buffer;
}

template<typename T>
static packet_data_t& serealize_args(packet_data_t& args_buffer
                                     , const T& value)
{
    return push_arg(args_buffer
                    , value);
}

template<typename T, typename... Targs>
static packet_data_t& serealize_args(packet_data_t& args_buffer
                                     , const T& value
                                     , Targs&& ...args)
{
    push_arg(args_buffer, value);
    return serealize_args(args_buffer
                         , args...);
}

template <typename T>
void deserialize_args(packet_data_t& buffer
                      , std::int32_t index
                      , T& value)
{
    pop_args(buffer, index, value);
}

template <typename T, typename... Targs>
void deserialize_args(packet_data_t& buffer
                      , std::int32_t index
                      , T& value
                      , Targs&& ...args)
{
    index = pop_args(buffer, index, value);
    deserialize_args(buffer, index, args...);
}

static void build_command_head(packet_data_t& command_buffer
                               , std::uint8_t address
                               , visca_command_t command)
{
    visca_header_t header{};

    header.start = 1;
    header.broadcast = static_cast<std::uint8_t>(address == 0);
    header.src_addr = 0;
    header.dst_addr = address;

    command_buffer.clear();

    auto command_size = get_command_size(command);

    command_buffer.resize(1 + command_size);

    command_buffer[0] = header.header;

    if (command_size > 0)
    {
        std::memcpy(command_buffer.data() + 1
                    , &command
                    , command_size);
    }

}

static packet_data_t& build_command(packet_data_t& command_buffer
                                    , std::uint8_t address
                                    , visca_command_t command)
{
    build_command_head(command_buffer
                       , address
                       , command);

    command_buffer.push_back(visca_eof);

    return command_buffer;
}

template<typename T>
static packet_data_t& build_command(packet_data_t& command_buffer
                                    , std::uint8_t address
                                    , visca_command_t command
                                    , const T& value)
{
    build_command_head(command_buffer
                       , address
                       , command);


    serealize_args(command_buffer
                   , value);

    command_buffer.push_back(visca_eof);

    return command_buffer;
}

template<typename T, typename... Targs>
static packet_data_t& build_command(packet_data_t& command_buffer
                                    , std::uint8_t address
                                    , visca_command_t command
                                    , const T& value
                                    , Targs&& ...args)
{
    build_command_head(command_buffer
                       , address
                       , command);


    serealize_args(command_buffer
                   , value
                   , args...);

    command_buffer.push_back(visca_eof);

    return command_buffer;
}

const serial::control_config_t visca_serial_config = { serial::baud_rate_t::br_9600
                                                     , serial::parity_t::p_none
                                                     , serial::stop_bits_t::sb_1
                                                     , serial::data_bits_t::db_8
                                                     , serial::flow_control_t::fc_no_flow};


struct visca_channel_t
{
    serial::serial_device&  port;
    packet_data_t           command_buffer;
    packet_data_t           response_buffer;
    response_parser_t       response_parser;
    response_packet_t       response;

    const visca_config_t&   config;

    visca_channel_t(serial::serial_device& port
                    , const visca_config_t& config)
        : port(port)
        , config(config)
    {

    }

    bool send_command(std::uint8_t address
                      , visca_command_t command)
    {
        if (is_init())
        {
            port.flush();

            build_command(command_buffer
                          , address
                          , command);

            auto write_size = port.write(command_buffer.data()
                                        , command_buffer.size());

            if (write_size > 0)
            {
                LOG_D << "Send packet: " <<  base::hex_dump(command_buffer.data()
                                                            , command_buffer.size()) LOG_END;

                return write_size == command_buffer.size();
            }
        }

        return false;
    }

    template<typename T>
    bool send_command(std::uint8_t address
                      , visca_command_t command
                      , const T& value)
    {
        if (is_init())
        {
            port.flush();

            build_command(command_buffer
                          , address
                          , command
                          , value);

            auto write_size = port.write(command_buffer.data()
                                        , command_buffer.size());

            if (write_size > 0)
            {
                LOG_D << "Send packet: " <<  base::hex_dump(command_buffer.data()
                                                            , command_buffer.size()) LOG_END;

                return write_size == command_buffer.size();
            }
        }

        return false;
    }

    template<typename T, typename... Targs>
    bool send_command(std::uint8_t address
                      , visca_command_t command
                      , const T& value
                      , Targs&& ...args)
    {
        if (is_init())
        {
            port.flush();

            build_command(command_buffer
                          , address
                          , command
                          , value
                          , args...);

            auto write_size = port.write(command_buffer.data()
                                        , command_buffer.size());

            if (write_size > 0)
            {
                LOG_D << "Send packet: " <<  base::hex_dump(command_buffer.data()
                                                            , command_buffer.size()) LOG_END;

                return write_size == command_buffer.size();
            }
        }

        return false;
    }

    bool get_replay(std::uint32_t address
                    , response_packet_t& response
                    , std::size_t response_data_size = 0)
    {

        if (is_init())
        {
            auto cur_tp = std::chrono::high_resolution_clock::now();
            auto end_tp = cur_tp + std::chrono::milliseconds(config.reply_timeout);

            std::this_thread::sleep_for(std::chrono::milliseconds(request_delay));
            response_parser.reset();

            do
            {
                auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(end_tp - cur_tp).count();

                response_buffer.clear();

                if (port.read(response_buffer
                              , false
                              , dt) > 0)
                {

                    LOG_D << "Recv packet: " <<  base::hex_dump(response_buffer.data()
                                                                , response_buffer.size()) LOG_END;

                    auto response_packets = response_parser.push_data(response_buffer.data()
                                                                      , response_buffer.size());

                    while(!response_packets.empty())
                    {
                        auto& current_response = response_packets.front();

                        if (current_response.address == address
                                && (response_data_size == 0
                                    || response_data_size == current_response.response_data.size()))
                        {
                            response = std::move(current_response);
                            return true;
                        }
                        response_packets.pop();
                    }
                }
                else
                {
                    break;
                }

                cur_tp = std::chrono::high_resolution_clock::now();
            }
            while(end_tp > cur_tp);

        }

        return false;
    }


    bool request(std::uint8_t address
                 , visca_command_t command
                 , std::size_t response_data_size = 0)
    {
        if (send_command(address
                         , command))
        {
            return get_replay(address
                              , response
                              , response_data_size);

        }

        return false;
    }

    template<typename T>
    bool request(std::uint8_t address
                 , visca_command_t command
                 , std::size_t response_data_size
                 , const T& value)
    {
        if (send_command(address
                         , command
                         , value))
        {
            return get_replay(address
                              , response
                              , response_data_size);

        }

        return false;
    }

    template<typename T, typename... Targs>
    bool request(std::uint8_t address
                 , visca_command_t command
                 , std::size_t response_data_size
                 , const T& value
                 , Targs&& ...args)
    {
        if (send_command(address
                         , command
                         , value
                         , args...))
        {
            return get_replay(address
                              , response
                              , response_data_size);

        }

        return false;
    }

    template<typename T>
    T fetch_response_arg(std::size_t offset = 0)
    {
        T result = {};

        if (offset + (sizeof(T))
                <= response.response_data.size())
        {
            result = reinterpret_cast<const T&>(*(response.response_data.data() + offset));
        }

        return result;
    }

    bool is_init() const
    {
        return port.is_opened();
    }

};



struct visca_device_context_t
{
    visca_config_t          m_config;
    serial::serial_device   m_port;
    std::uint8_t            m_address;
    visca_channel_t         m_channel;
    std::uint16_t           m_id;
    bool                    m_is_init;


    visca_device_context_t(const visca_config_t& config)
        : m_config(config)
        , m_address(1)
        , m_channel(m_port
                    , m_config)
        , m_id(0)
        , m_is_init(false)
    {

    }

    bool open(const std::string &uri)
    {
        m_is_init = false;

        if (m_port.open(uri))
        {
            set_address(m_address);
            get_id(m_id);

            m_is_init = true;

            /*if (!m_is_init)
            {
                m_port.close();
            }*/
        }

        return m_is_init;
    }

    bool close()
    {
        m_is_init = false;
        return m_port.close();
    }

    bool is_opened() const
    {
        return m_channel.is_init();
    }

    bool is_established() const
    {
        return is_opened()
                && m_is_init == true;
    }

    bool set_address(std::uint8_t address)
    {
        if (m_channel.request(0
                              , visca_command_t::cmd_address_set
                              , any_size
                              , address))
        {
            return true;
        }

        return false;
    }

    bool get_id(std::uint16_t& id)
    {
        if (m_channel.request(m_address
                              , visca_command_t::inq_cam_id
                              , 4))
        {
            id = from_visca_value(m_channel.fetch_response_arg<std::uint32_t>());

            return true;
        }

        return false;
    }

    bool set_pan_tilt(int16_t pan
                      , int16_t tilt)
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_pan_tilt_absolute
                              , any_size
                              , m_config.pan_speed
                              , m_config.tilt_speed
                              , to_visca_value(pan)
                              , to_visca_value(tilt)))
        {
            return true;
        }

        return false;
    }

    bool get_pan_tilt(int16_t& pan
                      , int16_t& tilt)
    {

        if (m_channel.request(m_address
                              , visca_command_t::inq_pan_tilt_pos
                              , 8))
        {
            pan = from_visca_value(m_channel.fetch_response_arg<std::uint32_t>(0));
            tilt = from_visca_value(m_channel.fetch_response_arg<std::uint32_t>(4));

            return true;
        }

        return false;
    }

    bool set_pan(int16_t pan)
    {
        std::int16_t dummy = 0;
        std::int16_t tilt = 0;

        return get_pan_tilt(dummy, tilt)
                && set_pan_tilt(pan, tilt);
    }

    bool set_tilt(int16_t tilt)
    {
        std::int16_t pan = 0;
        std::int16_t dummy = 0;

        return get_pan_tilt(pan, dummy)
                && set_pan_tilt(pan, tilt);
    }


    bool pan_tilt_reset()
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_pan_tilt_reset))
        {
            return true;
        }

        return false;
    }
    bool pan_tilt_home()
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_pan_tilt_home))
        {
            return true;
        }

        return false;
    }

    bool pan_tilt_stop()
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_pan_tilt_drive
                              , any_size
                              , 0x03030303))
        {
            return true;
        }

        return false;
    }

    bool set_zoom(uint16_t zoom)
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_cam_zoom_direct
                              , any_size
                              , to_visca_value(zoom)))
        {
            return true;
        }

        return false;
    }

    bool get_zoom(uint16_t &zoom)
    {
        if (m_channel.request(m_address
                              , visca_command_t::inq_cam_zoom_pos))
        {
            zoom = from_visca_value(m_channel.fetch_response_arg<std::uint32_t>());
            return true;
        }

        return false;
    }

    bool zoom_stop()
    {
        if (m_channel.request(m_address
                              , visca_command_t::cmd_cam_zoom
                              , 0
                              , static_cast<std::uint8_t>(0)))
        {
            return true;
        }

        return false;
    }

    bool get_ptz(double& pan, double& tilt, double& zoom)
    {
        std::int16_t v_pan = 0, v_tilt = 0;

        if (get_pan_tilt(v_pan, v_tilt))
        {
            std::uint16_t v_zoom = 0;

            if (get_zoom(v_zoom))
            {
                pan = static_cast<double>(v_pan - visca_pan_min) / static_cast<double>(visca_pan_range);
                tilt = static_cast<double>(v_tilt - visca_tilt_min) / static_cast<double>(visca_tilt_range);
                zoom = static_cast<double>(v_zoom - visca_zoom_min) / static_cast<double>(visca_zoom_range);

                return true;
            }
        }

        return false;
    }

    bool set_ptz(double pan, double tilt, double zoom)
    {
        if (set_pan_tilt(visca_pan_min + pan * visca_pan_range
                         , visca_tilt_min + tilt * visca_tilt_range))
        {
            return set_zoom(visca_zoom_min + zoom * visca_zoom_range);
        }

        return false;
    }


};

//---------------------------------------------------------------------------------------------
void visca_device_context_deleter_t::operator()(visca_device_context_t *visca_device_context_ptr)
{
    delete visca_device_context_ptr;
}
//---------------------------------------------------------------------------------------------
visca_device::visca_device(const visca_config_t& visca_config)
    : m_visca_device_context(new visca_device_context_t(visca_config))
{

}

bool visca_device::open(const std::string &uri)
{
    return m_visca_device_context->open(uri);
}

bool visca_device::close()
{
    return m_visca_device_context->close();
}

bool visca_device::is_opened() const
{
    return m_visca_device_context->is_opened();
}

bool visca_device::is_established() const
{
    return m_visca_device_context->is_established();
}

const visca_config_t &visca_device::config() const
{
    return m_visca_device_context->m_config;
}

bool visca_device::set_config(const visca_config_t &visca_config)
{
    m_visca_device_context->m_config = visca_config;
    return true;
}

bool visca_device::get_id(uint16_t &id)
{
    return m_visca_device_context->get_id(id);
}

bool visca_device::set_pan_tilt(int16_t pan
                                , int16_t tilt)
{
    return m_visca_device_context->set_pan_tilt(pan
                                                , tilt);
}

bool visca_device::set_pan(int16_t pan)
{
    return m_visca_device_context->set_pan(pan);
}

bool visca_device::set_tilt(int16_t tilt)
{
    return m_visca_device_context->set_tilt(tilt);
}

bool visca_device::get_pan_tilt(int16_t& pan
                                , int16_t& tilt)
{
    return m_visca_device_context->get_pan_tilt(pan
                                                , tilt);
}

bool visca_device::get_pan(int16_t &pan)
{
    std::int16_t tilt;
    return m_visca_device_context->get_pan_tilt(pan
                                                , tilt);
}

bool visca_device::get_tilt(int16_t &tilt)
{
    std::int16_t pan;
    return m_visca_device_context->get_pan_tilt(pan
                                                , tilt);
}

bool visca_device::pan_tilt_home()
{
    return m_visca_device_context->pan_tilt_home();
}

bool visca_device::pan_tilt_reset()
{
    return m_visca_device_context->pan_tilt_reset();
}

bool visca_device::pan_tilt_stop()
{
    return m_visca_device_context->pan_tilt_stop();
}

bool visca_device::set_zoom(uint16_t zoom)
{
    return m_visca_device_context->set_zoom(zoom);
}

bool visca_device::get_zoom(uint16_t &zoom)
{
    return m_visca_device_context->get_zoom(zoom);
}

bool visca_device::get_ptz(double &pan, double &tilt, double &zoom)
{
    return m_visca_device_context->get_ptz(pan, tilt, zoom);
}

bool visca_device::set_ptz(double pan, double tilt, double zoom)
{
    return m_visca_device_context->set_ptz(pan, tilt, zoom);
}

bool visca_device::zoom_stop()
{
    return m_visca_device_context->zoom_stop();
}

}
