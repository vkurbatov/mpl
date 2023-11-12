#include "tests.h"

#include "core/i_message_event.h"
#include "core/i_message_command.h"
#include "core/event_channel_state.h"
#include "core/i_message_sink.h"
#include "core/i_message_source.h"
#include "core/i_command_factory.h"
#include "core/i_event_factory.h"

#include "app/app_module_types.h"
#include "app/app_config.h"
#include "app/app_engine_impl.h"
#include "app/i_app_module.h"

#include "utils/pointer_utils.h"
#include "utils/time_utils.h"
#include "utils/enum_utils.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"
#include "utils/json_utils.h"
#include "utils/option_helper.h"

#include "media/media_module_types.h"
#include "media/audio_types.h"
#include "media/video_types.h"
#include "media/device_info.h"
#include "media/command_camera_control.h"
#include "media/i_media_stream.h"
#include "media/i_media_module.h"
#include "media/i_media_converter_collection.h"
#include "media/i_media_converter_factory.h"
#include "media/i_media_format_collection.h"
#include "media/i_media_format_factory.h"
#include "media/i_device_factory.h"
#include "media/i_device_factory_collection.h"
#include "media/mcu/layout_manager_mosaic_impl.h"
#include "media/audio_format_impl.h"
#include "media/video_format_impl.h"
#include "media/media_option_types.h"
#include "media/media_utils.h"

#include "net/net_module_types.h"
#include "net/socket/socket_endpoint.h"
#include "net/i_net_module.h"
#include "net/i_transport_collection.h"
#include "net/ice/ice_types.h"
#include "net/ice/ice_auth_params.h"
#include "net/ice/ice_transport_params.h"
#include "net/ice/ice_gathering_state_event.h"
#include "net/ice/ice_gathering_command.h"
#include "net/ice/i_ice_transport.h"

#include "log/log_tools.h"

#include <iostream>
#include <sstream>

namespace mpl::test
{

void ice_test()
{
    const net::ice_server_params_t::array_t ice_servers =
    {
        { "stun:stun1.l.google.com:19302" },
        { "stun:stun2.l.google.com:19302" },
        { "stun:stun3.l.google.com:19302" },
        { "stun:stun.l.google1.com:19302" }
    };

    app::app_config_t app_config;
    app_config.core_config.worker_count = 0;
    app_config.net_config.ice_config.auto_gathering = true;
    app_config.net_config.ice_config.ice_servers = ice_servers;

    if (app::i_app_engine::u_ptr_t app = app::app_engine_factory::get_instance().create_engine(app_config))
    {
        if (app->start())
        {

            if (net::i_net_module* net = static_cast<net::i_net_module*>(app->get_module(net::net_module_id)))
            {
                if (auto ice_factory = net->transports().get_factory(net::transport_id_t::ice))
                {
                    if (auto socket_packet_builder = net->create_packet_builder(net::transport_id_t::udp))
                    {
                        auto ice_params_1 = utils::property::create_property(property_type_t::object);
                        if (ice_params_1)
                        {

                            property_writer writer(*ice_params_1);
                            writer.append("sockets", net::udp_endpoint_t(net::socket_address_t::from_string("0.0.0.0:0")));
                            writer.set("mode", net::ice_mode_t::regular);
                            writer.set<std::uint8_t>("component_id", 1);
                            writer.set("local_endpoint.auth", net::ice_auth_params_t::generate());

                        }

                        auto ice_params_2 = utils::property::create_property(property_type_t::object);
                        if (ice_params_2)
                        {
                            property_writer writer(*ice_params_2);
                            writer.append("sockets", net::udp_endpoint_t(net::socket_address_t::from_string("0.0.0.0:0")));
                            writer.set("mode", net::ice_mode_t::aggressive);
                            writer.set<std::uint8_t>("component_id", 1);
                            writer.set("local_endpoint.auth", net::ice_auth_params_t::generate());
                        }

                        auto message_handler = [&](const std::string_view& name, const i_message& message)
                        {
                            switch(message.category())
                            {
                                case message_category_t::event:
                                {
                                    auto& message_event = static_cast<const i_message_event&>(message).event();
                                    switch(message_event.event_id)
                                    {
                                        case event_channel_state_t::id:
                                        {
                                            auto &event_channel_state = static_cast<const event_channel_state_t&>(message_event);
                                            std::cout << name
                                                      << ": channel state: " <<  utils::enum_to_string(event_channel_state.state)
                                                      << ", reason: " << event_channel_state.reason
                                                      << std::endl;
                                        }
                                        break;
                                        case net::ice_gathering_state_event_t::id:
                                        {
                                            auto &event_gathering_state = static_cast<const net::ice_gathering_state_event_t&>(message_event);
                                            std::cout << name
                                                      << ": gathering state: " <<  utils::enum_to_string(event_gathering_state.state)
                                                      << ", reason: " << event_gathering_state.reason
                                                      << std::endl;
                                        }
                                        break;
                                        default:;
                                    }
                                }
                                break;
                                case message_category_t::packet:
                                {
                                    if (message.module_id() == net::net_module_id)
                                    {
                                        auto& net_packet = static_cast<const net::i_net_packet&>(message);
                                        std::string_view text_message(static_cast<const char*>(net_packet.data())
                                                                                                , net_packet.size());
                                        std::cout << name
                                                  << ": packet: transport: " << utils::enum_to_string(net_packet.transport_id())
                                                  << ", size: " << net_packet.size()
                                                  << ", message: " << text_message
                                                  << std::endl;
                                    }
                                }
                                break;
                                default:;
                            }

                            return true;
                        };

                        message_sink_impl sink_1([&](auto&& ...args) { return message_handler("ice1", args...); } );
                        message_sink_impl sink_2([&](auto&& ...args) { return message_handler("ice2", args...); } );


                        net::i_ice_transport::u_ptr_t ice_connection_1 = utils::static_pointer_cast<net::i_ice_transport>(ice_factory->create_transport(*ice_params_1));
                        net::i_ice_transport::u_ptr_t ice_connection_2 = utils::static_pointer_cast<net::i_ice_transport>(ice_factory->create_transport(*ice_params_2));


                        ice_connection_1->source(0)->add_sink(&sink_1);
                        ice_connection_2->source(0)->add_sink(&sink_2);

                        ice_connection_1->control(channel_control_t::open());
                        ice_connection_2->control(channel_control_t::open());

                        utils::time::sleep(durations::seconds(1));

                        std::clog << "ice1 candidates:" << std::endl;
                        for (const auto& c : ice_connection_1->local_endpoint().candidates)
                        {
                            std::clog << c.to_string() << std::endl;
                        }

                        std::clog << "ice2 candidates:" << std::endl;
                        for (const auto& c : ice_connection_2->local_endpoint().candidates)
                        {
                            std::clog << c.to_string() << std::endl;
                        }

                        ice_connection_1->control(channel_control_t::connect());

                        utils::time::sleep(durations::seconds(2));
                        ice_connection_2->control(channel_control_t::connect());

                        const std::string_view test_string = "Kurbatov Vailiy Vladimirovich #";

                        while (true)
                        {
                            ice_connection_1->set_remote_endpoint(ice_connection_2->local_endpoint());
                            ice_connection_2->set_remote_endpoint(ice_connection_1->local_endpoint());

                            while(ice_connection_2->state() != channel_state_t::connected)
                            {
                                utils::time::sleep(durations::seconds(1));
                            }

                            for (auto i = 0; i < 10; i++)
                            {
                                auto test_message = std::string(test_string).append(std::to_string(i));

                                socket_packet_builder->set_packet_data(test_message.data()
                                                                       , test_message.size());

                                if (auto test_packet = socket_packet_builder->build_packet())
                                {
                                    ice_connection_1->sink(0)->send_message(*test_packet);
                                }

                                utils::time::sleep(durations::milliseconds(10));
                            }

                            ice_connection_2->control(channel_control_t::shutdown());

                            while(ice_connection_1->state() == channel_state_t::connected)
                            {
                                utils::time::sleep(durations::seconds(1));
                            }

                            ice_connection_2->control(channel_control_t::connect());

                            std::swap(ice_connection_1, ice_connection_2);
                        }

                        utils::time::sleep(durations::seconds(100));

                        ice_connection_2->control(channel_control_t::close());
                        ice_connection_1->control(channel_control_t::close());
                    }

                }
            }

            app->stop();
        }
    }

    return;
}

void visca_test()
{

    // socat -d -d pty,raw,echo=0 pty,raw,echo=0

    app::app_config_t app_config;
    app_config.core_config.worker_count = 0;
    app_config.net_config.ice_config.auto_gathering = true;
    app_config.net_config.listen_workers = 1;

    if (app::i_app_engine::u_ptr_t app = app::app_engine_factory::get_instance().create_engine(app_config))
    {
        if (app->start())
        {
            auto visca_params = property_helper::create_object();
            if (visca_params)
            {
                property_writer writer(*visca_params);
                writer.set("visca.reply_timeout_ms", 100);
                writer.set("visca.pan_speed", 10);
                writer.set("visca.tilt_speed", 10);
                writer.set("serial.baud_rate", 9600);
                writer.set("serial.char_size", 8);
                writer.set<std::string>("serial.parity", "even");
                writer.set<std::string>("serial.stop_bits", "one");
                writer.set<std::string>("serial.flow_control", "none");
                writer.set<std::string>("serial.endpoint.port_name", "/dev/pts/3");

            }

            auto visca_handler = [&](const i_message& message)
            {
                switch(message.category())
                {
                    case message_category_t::event:
                    {
                        auto& channel_state = static_cast<const event_channel_state_t&>(static_cast<const i_message_event&>(message).event());
                        std::cout << "Visca state: " << utils::enum_to_string(channel_state.state) << std::endl;
                    }
                    break;
                    case message_category_t::command:
                    {
                        auto& command_message = static_cast<const i_message_command&>(message);
                        if (command_message.command().command_id == media::command_camera_control_t::id)
                        {
                            auto& camera_control = static_cast<const media::command_camera_control_t&>(command_message.command());
                            if (camera_control.commands != nullptr)
                            {
                                auto json_controls = utils::to_json(*camera_control.commands, true);
                                std::cout << "json_controls: " << std::endl << json_controls << std::endl;
                            }

                            return true;
                        }
                    }
                    break;
                    default:;
                }

                return false;
            };

            app::i_app_module* app_module = static_cast<app::i_app_module*>(app->get_module(app::app_module_id));
            media::i_media_module* media_module = static_cast<media::i_media_module*>(app->get_module(media::media_module_id));
            auto visca_factory = media_module->devices().get_factory(media::device_type_t::visca);
            auto visca_device = visca_factory->create_device(*visca_params);

            message_sink_impl visca_sink(visca_handler);

            visca_device->source(0)->add_sink(&visca_sink);

            visca_device->control(channel_control_t::open());

            media::command_camera_control_t camera_control;
            camera_control.control_id = 123;

            utils::time::sleep(durations::seconds(1));

            visca_device->sink(0)->send_message(*app_module->commands().create_massage(camera_control));

            utils::time::sleep(durations::seconds(1));


            if (auto tilt_property = property_helper::create_object())
            {
                property_writer tilt_writer(*tilt_property);
                tilt_writer.set<std::string>("id", "tilt_absolute");
                tilt_writer.set<std::uint16_t>("value", 56);

                camera_control.commands = property_helper::create_array();
                auto& a = static_cast<i_property_array&>(*camera_control.commands);
                a.get_value().emplace_back(std::move(tilt_property));
                visca_device->sink(0)->send_message(*app_module->commands().create_massage(camera_control));
            }

            utils::time::sleep(durations::seconds(150));

            visca_device->control(channel_control_t::close());

            app->stop();
        }
    }

    // auto factory.create_device("");
}

void composer_test()
{
    app::app_config_t app_config;
    app_config.core_config.worker_count = 0;
    app_config.net_config.ice_config.auto_gathering = true;

    if (app::i_app_engine::u_ptr_t app = app::app_engine_factory::get_instance().create_engine(app_config))
    {
        if (app->start())
        {
            std::cout << "device list:" << std::endl;
            for (const auto& d : media::device_info_t::device_list())
            {
                std::cout << "type: " << utils::enum_to_string(d.media_type) << std::endl
                          << "\tname: " << d.name << std::endl
                          << "\tdesc: " << d.description << std::endl
                          << "\tclass: " << d.device_class << std::endl
                          << "\tdirection: " << utils::enum_to_string(d.direction) << std::endl
                          << std::endl;
            }


            if (media::i_media_module* media_module = static_cast<media::i_media_module*>(app->get_module(media::media_module_id)))
            {
                auto libav_input_device_factory = media_module->devices().get_factory(media::device_type_t::libav_in);
                auto v4l2_device_factory = media_module->devices().get_factory(media::device_type_t::v4l2_in);
                auto vnc_device_factory = media_module->devices().get_factory(media::device_type_t::vnc);
                auto libav_output_device_factory = media_module->devices().get_factory(media::device_type_t::libav_out);

                std::string bg_url = "/home/user/My/sportrecs/test_bckgrnd.mp4";
                //std::string bg_options = "rtsp_transport=tcp";

                std::string input_audio_url = "pulse://alsa_input.pci-0000_00_05.0.analog-stereo";
                std::string input_video_url = "/dev/video0";

                std::vector<std::string> input_urls =
                {
                    "/home/user/My/sportrecs/basket_streaming_video.mp4",
                    "/home/user/My/sportrecs/fight.mp4",
                    "/home/user/My/sportrecs/top-gun-maverick-trailer-3_h1080p.mov",
                    //"https://dagestan.mediacdn.ru/cdn/dagestan/playlist_hdhigh.m3u8"
                   /* "/home/user/My/sportrecs/sample.mp4",
                    "/home/user/My/sportrecs/basket_streaming_video.mp4",
                    "/home/user/My/sportrecs/fight.mp4",
                    "/home/user/My/sportrecs/top-gun-maverick-trailer-3_h1080p.mov",
                    "/home/user/My/sportrecs/sample.mp4"*/
                    "https://dagestan.mediacdn.ru/cdn/dagestan/playlist_hdhigh.m3u8"

                };


                std::string output_url = "rtmp://127.0.0.1/cam1/stream";

                auto libav_input_audio_params = property_helper::create_object();
                {
                    property_writer writer(*libav_input_audio_params);
                    writer.set<std::string>("url", input_audio_url);
                }

                auto v4l2_input_video_params = property_helper::create_object();
                {
                    property_writer writer(*v4l2_input_video_params);
                    writer.set<std::string>("url", input_video_url);
                }

                auto bg_video_params = property_helper::create_object();
                {
                    property_writer writer(*bg_video_params);
                    writer.set<std::string>("url", bg_url);
                }

                auto vnc_input_video_params = property_helper::create_object();
                {
                    property_writer writer(*vnc_input_video_params);
                    writer.set<std::string>("host", "192.168.0.103");
                    writer.set<std::uint16_t>("port", 5900);
                    writer.set<std::string>("password", "123123123");
                    writer.set<std::uint32_t>("fps", 30);
                }

                auto smart_factory = media_module->converters().get_factory(media::i_media_converter_collection::media_converter_type_t::smart);
                auto composer_factory = media_module->create_composer_factory(media::layout_manager_mosaic_impl::get_instance());
                // auto audio_format_factory =  media_module->formats().get_factory(media::media_type_t::audio);
                // auto video_format_factory =  media_module->formats().get_factory(media::media_type_t::video);

                auto audio_format_params = utils::property::create_property(property_type_t::object);
                auto video_format_params = utils::property::create_property(property_type_t::object);

                if (audio_format_params)
                {
                    property_writer writer(*audio_format_params);
                    writer.set("media_type", media::media_type_t::audio);
                    writer.set("format", media::audio_format_id_t::pcm16);
                    writer.set<std::uint32_t>("sample_rate", 48000);
                    writer.set<std::uint32_t>("channels", 2);
                }

                if (video_format_params)
                {
                    property_writer writer(*video_format_params);
                    writer.set("media_type", media::media_type_t::video);
                    writer.set("format", media::video_format_id_t::rgb24);
                    writer.set<std::uint32_t>("width", 1280);
                    writer.set<std::uint32_t>("height", 720);
                    writer.set<double>("frame_rate", 30);
                }

                auto composer_params = property_helper::create_object();

                if (composer_params)
                {
                    property_writer writer(*composer_params);
                    writer.set("audio_params.format", *audio_format_params);
                    writer.set("audio_params.duration", durations::milliseconds(10));
                    writer.set("video_params.format", *video_format_params);
                }

                auto media_composer = composer_factory->create_composer(*composer_params);

                auto stream_params = property_helper::create_object();

                std::size_t stream_count = input_urls.size() + 2;

                media::i_media_stream::s_array_t streams;

                double opacity = 1.0;

                for (std::size_t i = 0; i < stream_count; i++)
                {
                    property_writer writer(*stream_params);
                    writer.set("audio_track.enabled", true);
                    writer.set("video_track.enabled", true);
                    writer.set("audio_track.volume", 1.0);
                    writer.set("order", 1);
                    writer.set("video_track.draw_options.opacity", opacity);
                    // writer.set("video_track.draw_options.opacity", opacity);
                    writer.set("animation", 0.1);

                    std::string label = "stream #";
                    label.append(std::to_string(i));

                    writer.set("video_track.label", label);
                    writer.set<std::string>("video_track.user_img", "/home/user/test.jpg");

                    if (auto stream = media_composer->add_stream(*stream_params))
                    {
                        streams.emplace_back(std::move(stream));
                    }
                }

                if (stream_params)
                {
                    media::relative_frame_rect_t frame_rect = { 0.0, 0.0, 1.0, 1.0};
                    property_writer writer(*stream_params);
                    writer.set("order", 0);
                    writer.set("video_track.draw_options.rect", frame_rect);
                    writer.set<double>("video_track.draw_options.opacity", 1.0);
                    writer.remove("video_track.draw_options.label");
                    writer.remove("video_track.draw_options.elliptic");
                    writer.remove("video_track.draw_options.border");
                    writer.remove("video_track.user_img");
                    writer.remove("video_track.animation");

                }

                auto stream10 = media_composer->add_stream(*stream_params);

                {
                    property_writer writer(*audio_format_params);
                    writer.set("format", media::audio_format_id_t::aac);
                }

                {
                    property_writer writer(*video_format_params);
                    writer.set("format", media::video_format_id_t::h264);
                }

                auto transcode_audio_params = utils::property::create_property(property_type_t::object);
                auto transcode_video_params = utils::property::create_property(property_type_t::object);

                /*
                auto transcode_audio_params = audio_format_params->clone();
                auto transcode_video_params = video_format_params->clone();*/

                std::string encoder_options = "profile=baseline;preset=ultrafast;tune=zerolatency;cfr=22;g=60;keyint_min=30;max_delay=0;bf=0;threads=4";

                {
                    property_writer writer(*transcode_audio_params);
                    writer.set("format", *audio_format_params);
                    writer.set("transcode_async", true);
                    option_impl options;
                    option_writer(options).set(media::opt_frm_track_id, media::default_audio_track_id);
                    utils::convert_format_options(options, *writer["format"]);

                }

                {
                    property_writer writer(*transcode_video_params);
                    writer.set("format", *video_format_params);
                    option_impl options;
                    option_writer(options).set(media::opt_frm_track_id, media::default_video_track_id);
                    option_writer(options).set(media::opt_codec_params, encoder_options);
                    utils::convert_format_options(options, *writer["format"]);
                }


                auto audio_transcoder = smart_factory->create_converter(*transcode_audio_params);


                auto video_transcoder = smart_factory->create_converter(*transcode_video_params);

                auto libav_output_device_params = property_helper::create_object();
                {
                    property_writer writer(*libav_output_device_params);
                    writer.set<std::string>("url", output_url);
                    i_property::s_array_t streams;

                    if (auto vp = video_format_params->clone())
                    {
                        streams.emplace_back(std::move(vp));
                    }

                    if (auto ap = audio_format_params->clone())
                    {
                        streams.emplace_back(std::move(ap));
                    }

                    writer.set("streams", streams);
                }

                auto input_audio_device = libav_input_device_factory->create_device(*libav_input_audio_params);
                auto input_video_device = v4l2_device_factory->create_device(*v4l2_input_video_params);
                auto bg_video_device = libav_input_device_factory->create_device(*bg_video_params);
                auto vnc_device = vnc_device_factory->create_device(*vnc_input_video_params);
                auto output_device = libav_output_device_factory->create_device(*libav_output_device_params);

                std::vector<media::i_device::s_ptr_t> devices;

                auto i = 2;

                for (const auto& url : input_urls)
                {
                    if (auto device_params = property_helper::create_object())
                    {
                        property_writer writer(*device_params);
                        writer.set<std::string>("url", url);

                        if (auto device = libav_input_device_factory->create_device(*device_params))
                        {
                            device->source(0)->add_sink(streams[i]->sink(0));
                            devices.emplace_back(std::move(device));
                            i++;
                        }
                    }
                }

                bg_video_device->source(0)->add_sink(stream10->sink(0));

                vnc_device->source(0)->add_sink(streams[1]->sink(0));

                input_audio_device->source(0)->add_sink(streams[0]->sink(0));
                input_video_device->source(0)->add_sink(streams[0]->sink(0));


                streams[0]->source(0)->add_sink(video_transcoder.get());
                streams[0]->source(0)->add_sink(audio_transcoder.get());


                audio_transcoder->set_sink(output_device->sink(0));
                video_transcoder->set_sink(output_device->sink(0));

                media_composer->start();

                output_device->control(channel_control_t::open());
                input_audio_device->control(channel_control_t::open());
                input_video_device->control(channel_control_t::open());
                // vnc_device->control(channel_control_t::open());
                bg_video_device->control(channel_control_t::open());

                for (auto& d : devices)
                {
                    d->control(channel_control_t::open());
                }

                while(input_video_device->state() != channel_state_t::connected);

                std::size_t count = 1000;

                auto sp = property_helper::create_object();

                if (stream10->get_params(*sp))
                {
                    property_writer writer(*sp);
                    writer.set("order", 1);
                    // stream10->set_params(*sp);
                }

                utils::time::sleep(durations::seconds(120));


                /*
                while(count-- > 0)
                {
                    for (auto& s : streams)
                    {
                        if (s->stream_id() % 2 == 0)
                        {
                            continue;
                        }

                        auto stream_property = property_helper::create_object();
                        if (count % 10 == 0)
                        {
                            if (s->get_params(*stream_property))
                            {
                                property_writer writer(*stream_property);
                                writer.set("audio_enabled", false);
                                writer.set("video_enabled", false);

                                s->set_params(*stream_property);
                            }
                        }
                        else if (count % 5 == 0)
                        {
                            if (s->get_params(*stream_property))
                            {
                                property_writer writer(*stream_property);
                                writer.set("audio_enabled", true);
                                writer.set("video_enabled", true);

                                s->set_params(*stream_property);
                            }
                        }
                    }
                    core::utils::sleep(durations::seconds(1));
                }
                */

                media_composer->stop();

                input_audio_device->control(channel_control_t::close());
                input_video_device->control(channel_control_t::close());
                bg_video_device->control(channel_control_t::close());
                vnc_device->control(channel_control_t::close());

                for (auto& d : devices)
                {
                    d->control(channel_control_t::close());
                }

                output_device->control(channel_control_t::close());

                audio_transcoder->set_sink(nullptr);
                video_transcoder->set_sink(nullptr);
            }

        app->stop();

        }
    }

    return;
}

// Terminator
void log_recursive(const char* file, int line, std::ostringstream& msg)
{
    std::cout << file << "(" << line << "): " << msg.str() << std::endl;
}

template<typename T, typename... Args>
void log_recursive(const char* file, int line, std::ostringstream& msg,
                   T value, const Args&... args)
{
    msg << value;
    log_recursive(file, line, msg, args...);
}

#define LOG(...) log_wrapper(__FILE__, __LINE__, __VA_ARGS__)

template<typename ...Args>
void log_wrapper(const char* file, int line, Args&& ...args)
{
    std::ostringstream msg;
    log_recursive(file, line, msg, args...);
}


void log_test()
{
    mpl::log::set_log_level(log::log_level_t::info);

    mpl_log_trace("message ", 1, " trace");
    mpl_log_debuf("message ", 2, " debug");
    mpl_log_error("message ", 3, " error");
    mpl_log_info("message ", 4, " info");
    mpl_log_fatal("message ", 5, " fatal");
    mpl_log_warning("message ", 6, " warning");
}

void tests()
{
    ice_test();
    // visca_test();
    // composer_test();
    // log_test();
}

}
