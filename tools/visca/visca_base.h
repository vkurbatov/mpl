#ifndef VISCA_BASE_H
#define VISCA_BASE_H

#include <string>
#include <vector>
#include <queue>

namespace visca
{

typedef std::vector<std::uint8_t> packet_data_t;

const std::uint8_t default_reply_timeout = 1000;
const std::uint8_t default_pan_speed = 0x10;
const std::uint8_t default_tilt_speed = 0x10;

const std::int32_t visca_pan_min = -2448;
const std::int32_t visca_pan_max = 2448;
const std::int32_t visca_tilt_min = -432;
const std::int32_t visca_tilt_max = 1296;
const std::int32_t visca_zoom_min = 0;
const std::int32_t visca_zoom_max = 16384;
const std::int32_t visca_pan_range = visca_pan_max - visca_pan_min;
const std::int32_t visca_tilt_range = visca_tilt_max - visca_tilt_min;
const std::int32_t visca_zoom_range = visca_zoom_max - visca_zoom_min;


const std::uint8_t visca_eof = 0xff;

enum class response_type_t
{
    unknown = 0,
    address = 3,
    ack = 4,
    completed = 5,
    error = 6
};

enum class parse_state_t
{
    header,
    response_type,
    terminator
};

enum class visca_command_id_t
{
    direct_mask             = 0x400000,
    on_off_mask             = 0x300000,
    cmd_custom              = 0,
    cmd_address_set         = 0x30,
    cmd_if_clear            = 0x010001,
    cmd_cancel              = 0x21,
    cmd_cam_power           = 0x000401,
    cmd_cam_zoom            = 0x070401,
    cmd_cam_zoom_direct     = cmd_cam_zoom | direct_mask,
    cmd_cam_focus           = 0x080401,
    cmd_cam_zfocus          = 0x470401,
    cmd_cam_wb              = 0x350401,
    cmd_cam_rgain           = 0x030401,
    cmd_cam_rgain_direct    = cmd_cam_rgain | direct_mask,
    cmd_cam_bgain           = 0x040401,
    cmd_cam_bgain_direct    = cmd_cam_bgain | direct_mask,
    cmd_cam_ae              = 0x390401,
    cmd_cam_shutter         = 0x0a0401,
    cmd_cam_shutter_direct  = cmd_cam_shutter | direct_mask,
    cmd_cam_iris            = 0x0b0401,
    cmd_cam_iris_direct     = cmd_cam_iris | direct_mask,
    cmd_cam_gain            = 0x0c0401,
    cmd_cam_gain_direct     = cmd_cam_gain | direct_mask,
    cmd_cam_bright          = 0x0d0401,
    cmd_cam_bright_direct   = cmd_cam_bright | direct_mask,
    cmd_cam_exp_comp        = 0x0e0401,
    cmd_cam_exp_comp_swith  = cmd_cam_exp_comp | on_off_mask,
    cmd_cam_exp_comp_direct = cmd_cam_exp_comp | direct_mask,
    cmd_pan_tilt_drive      = 0x010601,
    cmd_pan_tilt_absolute   = 0x020601,
    cmd_pan_tilt_relative   = 0x030601,
    cmd_pan_tilt_home       = 0x040601,
    cmd_pan_tilt_reset      = 0x050601,
    inq_cam_id              = 0x220409,
    inq_cam_version         = 0x020009,
    inq_cam_zoom_pos        = 0x470409,
    inq_pan_tilt_pos        = 0x120609,
};

union visca_header_t
{
    struct
    {
        std::uint8_t    dst_addr:3;
        std::uint8_t    broadcast:1;
        std::uint8_t    src_addr:3;
        std::uint8_t    start:1;
    };

    std::uint8_t        header;
};

struct response_packet_t
{
    std::uint8_t        address;
    response_type_t     type;
    packet_data_t       response_data;

    response_packet_t(std::uint8_t address = 0
                      , response_type_t type = response_type_t::unknown
                      , packet_data_t&& response_data = {});

    void clear();
};

typedef std::queue<response_packet_t> packet_queue_t;

struct response_parser_t
{
    response_packet_t   current_packet;
    parse_state_t       parse_state;

    response_parser_t();

    packet_queue_t push_data(const void* data
                             , std::size_t size);
    void reset();


};

struct visca_config_t
{
    std::uint32_t   reply_timeout;
    std::uint8_t    pan_speed;
    std::uint8_t    tilt_speed;

    visca_config_t(std::uint32_t reply_timeout = default_reply_timeout
                   , std::uint8_t pan_speed = default_pan_speed
                   , std::uint8_t tilt_speed = default_tilt_speed);
};

}


#endif // VISCA_BASE_H
