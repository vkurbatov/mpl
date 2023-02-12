#include <iostream>
#include "test.h"
#include "core/convert_utils.h"
#include "core/enum_utils.h"
#include "core/time_utils.h"
#include "core/property_value_impl.h"
#include "core/property_tree_impl.h"

#include "core/property_writer.h"

#include "core/convert_utils.h"
#include "audio_format_impl.h"
#include "video_format_impl.h"
#include "tools/base/any_base.h"

#include "core/option_helper.h"

#include "v4l2_device_factory.h"
#include "libav_input_device_factory.h"
#include "libav_output_device_factory.h"
#include "core/message_sink_impl.h"
#include "core/i_buffer_collection.h"
#include "i_audio_frame.h"
#include "i_audio_format.h"
#include "i_video_frame.h"
#include "i_video_format.h"

#include "libav_audio_converter_factory.h"
#include "libav_video_converter_factory.h"
#include "media_converter_factory_impl.h"
#include "libav_transcoder_factory.h"
#include "smart_transcoder_factory.h"

#include "media_option_types.h"


#include "i_message_frame.h"
#include "core/i_message_event.h"
#include "core/i_message_source.h"

#include "core/event_channel_state.h"

#include "tools/ffmpeg/libav_base.h"
#include "tools/ffmpeg/libav_stream_grabber.h"
#include "tools/ffmpeg/libav_stream_publisher.h"
#include "tools/base/url_base.h"

#include <string>

namespace mpl::media
{

namespace
{

void test1()
{
    auto test_tree = property_helper::create_tree();
    property_writer writer(*test_tree);

    writer.set<std::string>("name.first", "Vasiliy");
    writer.set<std::string>("name.second", "Kurbatov");
    writer.set<std::int32_t>("name.age", 40);

    property_reader reader(*test_tree);

    std::cout << *reader.get<std::string>("name.first") << " "
              << *reader.get<std::string>("name.second") << " "
              << *reader.get<std::int32_t>("name.age") << " "
              << std::endl;

}

void test2()
{
    auto convert_test = [](auto& in, auto& out)
    {
        if (core::utils::convert(in, out))
        {
            std::cout << "forward conversion: from " << in << " to " << out << " completed" << std::endl;
            if (core::utils::convert(out, in))
            {
                std::cout << "backward conversion: from " << out << " to " << in << " completed" << std::endl;
            }
        }
    };

    std::int8_t i8_value = 1;
    std::int16_t i16_value = 2;
    std::int32_t i32_value = 3;
    std::int64_t i64_value = 4;
    std::uint8_t u8_value = 5;
    std::uint16_t u16_value = 6;
    std::uint32_t u32_value = 7;
    std::uint64_t u64_value = 8;
    float r32_value = 9.0f;
    double r64_value = 10.0;
    long double r96_value = 11.0;
    std::string s_value = "12";
    std::vector<std::uint8_t> hex_test = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

    std::string test_string;

    convert_test(i8_value, test_string);
    convert_test(i16_value, test_string);
    convert_test(i32_value, test_string);
    convert_test(i64_value, test_string);
    convert_test(u8_value, test_string);
    convert_test(u16_value, test_string);
    convert_test(u32_value, test_string);
    convert_test(u64_value, test_string);
    convert_test(r32_value, test_string);
    convert_test(r64_value, test_string);
    convert_test(r96_value, test_string);
    convert_test(s_value, test_string);

    test_string.clear();

    if (core::utils::convert(hex_test, test_string))
    {
        std::cout << "forward conversion hex " << test_string << " completed" << std::endl;
        if (core::utils::convert(test_string, hex_test))
        {
            std::cout << "backward conversion hex " << test_string << " completed" << std::endl;
        }
    }



    return;
}

void test3()
{
    base::any any1, any2, any3;

    any1 = 1;
    base::any any11 = any1;
    any2 = 4.567;
    base::any any22(any2);
    any3 = std::string("vasiliy");
    auto any33(any3);


    auto cmp1 = any1 == any2;
    auto cmp2 = any2 == any3;
    auto cmp3 = any3 == any1;

    auto cmp4 = any1 == any11;
    auto cmp5 = any2 == any22;
    auto cmp6 = any3 == any33;

    auto v1 = any1.cast<std::int32_t>();
    auto v2 = any2.cast<double>();
    auto v3 = any3.cast<std::string>();

    auto pv1 = any1.cast<std::int32_t*>();
    auto pv2 = any2.cast<double*>();
    auto pv3 = any3.cast<std::string*>();

    auto r1 = std::is_pointer<std::int32_t>::value;
    auto r2 = std::is_pointer<std::int32_t*>::value;

    return;

    /*
    audio_format_impl audio_format(audio_format_id_t::opus, 48000, 2);
    auto clone_format = audio_format.clone();
    auto& a_clone = static_cast<i_audio_format&>(*clone_format);
    auto& a_clone2 = static_cast<i_media_format&>(audio_format);

    auto cpb = audio_format.is_compatible(*clone_format);

    auto eq = audio_format.is_equal(*clone_format);

    return;
*/
}

void test4()
{

    option_impl option1;
    option_impl option2;

    option_writer writer1(option1);
    option_writer writer2(option2);


    writer1.set(1, 123);
    writer1.set(2, 456.7);
    writer1.set(3, true);
    writer1.set(4, std::string("Vasiliy"));

    writer2.set(5, 321);
    writer2.set(6, 7.654);
    writer2.set(7, false);
    writer2.set(8, std::string("Kurbatov"));

    auto cmp1 = option1.is_equal(option2);
    auto c1 = option1.merge(option2);
    auto c2 = option2.merge(option1);
    auto cmp2 = option1.is_equal(option2);


    return;
}

void test5()
{
    std::string v4l2_url = "/dev/video0";
    auto v4l2_params = property_helper::create_tree();
    property_writer writer(*v4l2_params);
    writer.set<std::string>("url", v4l2_url);

    v4l2_device_factory factory;

    if (auto device = factory.create_device(*v4l2_params))
    {
        auto handler = [&](const i_message& message)
        {
            switch(message.category())
            {
                case message_category_t::frame:
                {
                    const auto& frame_message = static_cast<const i_message_frame&>(message);

                    if (frame_message.frame().media_type() == media_type_t::video)
                    {
                        const auto& video_frame = static_cast<const i_video_frame&>(frame_message.frame());
                        std::cout << "on_frame #" << video_frame.frame_id()
                                  << ": format_id: " << static_cast<std::int32_t>(video_frame.format().format_id())
                                  << ", fmt: " << video_frame.format().width()
                                  << "x" << video_frame.format().height()
                                  << "@" << video_frame.format().frame_rate()
                                  << ", ts: " << video_frame.timestamp();

                        if (auto buffer = video_frame.buffers().get_buffer(0))
                        {
                            std::cout << ", size: " << buffer->size();
                        }

                        std::cout << std::endl;

                    }
                }
                break;
                case message_category_t::event:
                {
                    const auto& event_message = static_cast<const i_message_event&>(message);
                    if (event_message.event().event_id == event_id_t::channel_state)
                    {
                        const auto& channel_state = static_cast<const event_channel_state_t&>(event_message.event());
                        std::cout << "device state: " << static_cast<std::uint32_t>(channel_state.state) << std::endl;
                    }
                }
                break;
                default:;
            }

            return false;
        };

        message_sink_impl sink(handler);
        device->source()->add_sink(&sink);

        if (device->control(channel_control_t::open()))
        {

            core::utils::sleep(durations::seconds(60));
            device->control(channel_control_t::close());
        }

        device->source()->remove_sink(&sink);

    }

    return;
}

void test6()
{
    device_type_t enum_value = device_type_t::undefined;
    std::string string_value;
    core::utils::convert(device_type_t::libav_in, string_value);
    core::utils::convert(string_value, enum_value);
    auto s2 = core::utils::enum_to_string<device_type_t>(enum_value);
    auto e2 = core::utils::string_to_enum<device_type_t>(s2);

    auto tree = property_helper::create_tree();
    property_writer writer(*tree);

    writer.set("device_type", device_type_t::v4l2_in);
    auto e3 = writer.get<device_type_t>("device_type");

    return;
}

struct ip_endpoint_t
{
    std::uint32_t address;
    std::uint16_t port;

    ip_endpoint_t() {}

    ip_endpoint_t(std::uint32_t address = 0
                  , std::uint16_t port = 0)
        : address(address)
        , port(port)
    {

    }

    bool operator == (const ip_endpoint_t& other) const
    {
        return true;
    }

    bool operator != (const ip_endpoint_t& other) const
    {
        return !operator ==(other);
    }
};

struct A
{

};

void test7()
{
    ip_endpoint_t endpoint1(1234567, 1234);
    ip_endpoint_t endpoint2(endpoint1);

    option_impl options;

    std::string test_string = "vasiliy";
    std::int32_t test_int = 0;

    base::any any_ep(test_int);

    std::any any_test(endpoint1);

    const ip_endpoint_t* e = std::any_cast<ip_endpoint_t>(&any_test);


    // options.set(0, endpoint2);
    /*if (auto e = options.get(0).cast<const ip_endpoint_t*>())
    {
        return;
    }*/

    return;
}

void test8()
{

    ffmpeg::libav_register();

    std::string libav_url = "rtsp://wowzaec2demo.streamlock.net/vod/mp4";
    std::string libav_options = "rtsp_transport=tcp";
    auto libav_params = property_helper::create_tree();
    property_writer writer(*libav_params);
    writer.set<std::string>("url", libav_url);
    writer.set<std::string>("options", libav_options);

    libav_input_device_factory factory;

    libav_audio_converter_factory audio_converter_factory;
    libav_video_converter_factory video_converter_factory;
    media_converter_factory_impl media_converter_factory(audio_converter_factory
                                                         , video_converter_factory);

    libav_transcoder_factory decoder_factory(false);
    libav_transcoder_factory encoder_factory(true);


    smart_transcoder_factory smart_factory(decoder_factory
                                           , encoder_factory
                                           , media_converter_factory);


    if (auto device = factory.create_device(*libav_params))
    {
        auto handler = [&](const i_message& message)
        {
            switch(message.category())
            {
                case message_category_t::frame:
                {
                    const auto& frame_message = static_cast<const i_message_frame&>(message);

                    if (frame_message.frame().media_type() == media_type_t::video)
                    {
                        const auto& video_frame = static_cast<const i_video_frame&>(frame_message.frame());
                        std::cout << "on_frame #" << video_frame.frame_id()
                                  << ": format_id: " << core::utils::enum_to_string(video_frame.format().format_id())
                                  << ", fmt: " << video_frame.format().width()
                                  << "x" << video_frame.format().height()
                                  << "@" << video_frame.format().frame_rate()
                                  << ", ts: " << video_frame.timestamp()
                                  << ", kf: " << (video_frame.frame_type() == i_video_frame::frame_type_t::key_frame);

                        if (auto buffer = video_frame.buffers().get_buffer(0))
                        {
                            std::cout << ", size: " << buffer->size();
                        }

                        std::cout << std::endl;

                    }
                    else if (frame_message.frame().media_type() == media_type_t::audio)
                    {
                        const auto& audio_frame = static_cast<const i_audio_frame&>(frame_message.frame());
                        std::cout << "on_frame #" << audio_frame.frame_id()
                                  << ": format_id: " << core::utils::enum_to_string(audio_frame.format().format_id())
                                  << ", fmt: " << audio_frame.format().sample_rate()
                                  << "/" << audio_frame.format().channels()
                                  << ", ts: " << audio_frame.timestamp();

                        if (auto buffer = audio_frame.buffers().get_buffer(0))
                        {
                            std::cout << ", size: " << buffer->size();
                        }

                        std::cout << std::endl;
                    }
                }
                break;
                case message_category_t::event:
                {
                    const auto& event_message = static_cast<const i_message_event&>(message);
                    if (event_message.event().event_id == event_id_t::channel_state)
                    {
                        const auto& channel_state = static_cast<const event_channel_state_t&>(event_message.event());
                        std::cout << "device state: " << static_cast<std::uint32_t>(channel_state.state) << std::endl;
                    }
                }
                break;
                default:;
            }

            return true;
        };

        audio_format_impl audio_format(audio_format_id_t::pcma
                                       , 48000
                                       , 2);
        video_format_impl video_format (video_format_id_t::h264
                                        , 1280
                                        , 720
                                        , 30);

        std::string encoder_options = "profile=baseline;preset=ultrafast;tune=zerolatency;cfr=22;g=60;keyint_min=30;max_delay=0;bf=0;threads=4";

        option_writer(video_format.options()).set(opt_codec_params, encoder_options);

        auto audio_transcoder = smart_factory.create_converter(audio_format);
        auto video_transcoder = smart_factory.create_converter(video_format);

        message_sink_impl sink(handler);

        audio_transcoder->set_sink(&sink);
        video_transcoder->set_sink(&sink);

        device->source()->add_sink(audio_transcoder.get());
        device->source()->add_sink(video_transcoder.get());

        if (device->control(channel_control_t::open()))
        {

            core::utils::sleep(durations::seconds(60));
            device->control(channel_control_t::close());
        }

        device->source()->remove_sink(audio_transcoder.get());
        device->source()->remove_sink(video_transcoder.get());
    }

    return;
}

void test9()
{
    ffmpeg::libav_register();

    std::string input_url = "rtsp://wowzaec2demo.streamlock.net/vod/mp4";
    std::string input_options = "rtsp_transport=tcp";

    std::string output_url = "rtmp://127.0.0.1/cam1/stream";
    auto libav_input_device_params = property_helper::create_tree();
    {
        property_writer writer(*libav_input_device_params);
        writer.set<std::string>("url", input_url);
        writer.set<std::string>("options", input_options);
    }

    auto libav_output_device_params = property_helper::create_tree();
    {
        property_writer writer(*libav_output_device_params);
        writer.set<std::string>("url", output_url);
        audio_format_impl audio_format(audio_format_id_t::aac
                                       , 48000
                                       , 2);

        video_format_impl video_format(video_format_id_t::h264
                                       , 512
                                       , 288
                                       , 30);

        option_writer(video_format.options()).set<std::int32_t>(opt_fmt_stream_id, 1);

        i_property::array_t streams;
        if (auto ap = property_helper::create_tree())
        {
            if (audio_format.get_params(*ap))
            {
                streams.emplace_back(std::move(ap));
            }
        }

        if (auto vp = property_helper::create_tree())
        {
            if (video_format.get_params(*vp))
            {
                streams.emplace_back(std::move(vp));
            }
        }

        writer.set("streams", streams);
    }

    libav_input_device_factory input_device_factory;
    libav_output_device_factory output_device_factory;


    if (auto input_device = input_device_factory.create_device(*libav_input_device_params))
    {
        if (auto output_device = output_device_factory.create_device(*libav_output_device_params))
        {
            input_device->source()->add_sink(output_device->sink());

            if (output_device->control(channel_control_t::open()))
            {

                if (input_device->control(channel_control_t::open()))
                {
                    core::utils::sleep(durations::seconds(60));
                    input_device->control(channel_control_t::close());
                }

                output_device->control(channel_control_t::close());
            }

            input_device->source()->remove_sink(output_device->sink());
        }
    }

    return;
}

void test10()
{
    ffmpeg::libav_register();

    auto audio_input_class_list = ffmpeg::device_info_t::device_class_list(ffmpeg::media_type_t::audio, true);
    auto audio_output_class_list = ffmpeg::device_info_t::device_class_list(ffmpeg::media_type_t::audio, false);
    auto video_input_class_list = ffmpeg::device_info_t::device_class_list(ffmpeg::media_type_t::video, true);
    auto video_output_class_list = ffmpeg::device_info_t::device_class_list(ffmpeg::media_type_t::video, false);

    std::cout << "Audio input class list: " << std::endl;
    for (const auto& c : audio_input_class_list)
    {
        std::cout << c << std::endl;
    }

    std::cout << "Audio output class list: " << std::endl;
    for (const auto& c : audio_output_class_list)
    {
        std::cout << c << std::endl;
    }

    std::cout << "Video input class list: " << std::endl;
    for (const auto& c : video_input_class_list)
    {
        std::cout << c << std::endl;
    }

    std::cout << "Video output class list: " << std::endl;
    for (const auto& c : video_output_class_list)
    {
        std::cout << c << std::endl;
    }

    auto audio_input_device_list = ffmpeg::device_info_t::device_list(ffmpeg::media_type_t::audio
                                                                      , true
                                                                      , "pulse");

    auto audio_output_device_list = ffmpeg::device_info_t::device_list(ffmpeg::media_type_t::audio
                                                                      , false
                                                                      , "pulse");
    /*    std::string name;
    std::string description;
    std::string device_class;
    bool        is_source;*/

    std::cout << "Audio input device list: " << std::endl;
    for (const auto& d : audio_input_device_list)
    {
        std::cout << "Name: " << d.name
                  << ", Description: " << d.description
                  << ", Class: " << d.device_class
                  << ", Direction: " << d.is_source
                  << ", Uri: " << d.to_uri()
                  << std::endl;
    }

    std::cout << "Audio output device list: " << std::endl;
    for (const auto& d : audio_output_device_list)
    {
        std::cout << "Name: " << d.name
                  << ", Description: " << d.description
                  << ", Class: " << d.device_class
                  << ", Direction: " << d.is_source
                  << ", Uri: " << d.to_uri()
                  << std::endl;
    }


    std::string output_url = "pulse://alsa_output.pci-0000_00_05.0.analog-stereo";
    ffmpeg::libav_stream_publisher libav_publisher;
    ffmpeg::stream_info_t stream_info;
    stream_info.codec_info.id = ffmpeg::codec_id_none;
    stream_info.stream_id = 0;
    stream_info.media_info.media_type = ffmpeg::media_type_t::audio;
    stream_info.media_info.audio_info.sample_format = ffmpeg::sample_format_pcm16;
    stream_info.media_info.audio_info.sample_rate = 48000;
    stream_info.media_info.audio_info.channels = 2;

    auto r = libav_publisher.open(output_url
                                  , {stream_info});




    auto frame_handler = [&](const ffmpeg::stream_info_t& stream_info
            , ffmpeg::frame_t&& frame) ->
    bool
    {
        std::cout << "Frame #" << frame.info.id << std::endl;

        if (libav_publisher.is_opened())
        {
            frame.info.id = 0;
            // libav_publisher.push_frame(frame);
        }
        return true;
    };

    auto event_handler = [](const ffmpeg::streaming_event_t& streaming_event) ->
    void
    {
        std::cout << "device event: " << static_cast<std::int32_t>(streaming_event) << std::endl;
    };

    ffmpeg::libav_stream_grabber libav_grabber(frame_handler
                                               , event_handler);

    ffmpeg::libav_grabber_config_t config;
    config.stream_mask = ffmpeg::stream_mask_all;
    // config.url = "alsa://hw:0,0";
    config.url = "pulse://alsa_input.pci-0000_00_05.0.analog-stereo";
    libav_grabber.open(config.url);


    core::utils::sleep(durations::seconds(60));

    return;
}

void test11()
{
    const std::vector<std::string> test_urls =
    {
        "rtsp://192.168.0.1",
        "rtsp://user@192.168.0.1",
        "rtsp://user:pass@192.168.0.1",
        "rtsp://user:pass@192.168.0.1:1234",
        "rtsp://user:pass@192.168.0.1:1234?param1=1",
        "rtsp://user:pass@192.168.0.1?param1=1",
    };

    for (const auto& u : test_urls)
    {
        std::cout << "Parse URL: " << u << std::endl;
        base::url_info_t url;
        if (url.parse_url(u))
        {
            std::cout << "Parse Completed! " << std::endl;
            std::cout << "Reverse build url: " << url.to_url() << std::endl;
        }
        else
        {
            std::cout << "Parse Failed! " << std::endl;
        }
    }

    return;
}

void test12()
{
    ffmpeg::libav_register();
    libav_input_device_factory input_device_factory;

    std::string input_url = "pulse://alsa_input.pci-0000_00_05.0.analog-stereo";

    //std::string input_options = "rtsp_transport=tcp";

    auto libav_input_device_params = property_helper::create_tree();
    {
        property_writer writer(*libav_input_device_params);
        writer.set<std::string>("url", input_url);
        // writer.set<std::string>("options", input_options);
    }

    if (auto input_device = input_device_factory.create_device(*libav_input_device_params))
    {
        auto handler = [&](const i_message& message)
        {
            switch(message.category())
            {
                case message_category_t::frame:
                {
                    const auto& frame_message = static_cast<const i_message_frame&>(message);

                    if (frame_message.frame().media_type() == media_type_t::video)
                    {
                        const auto& video_frame = static_cast<const i_video_frame&>(frame_message.frame());
                        std::cout << "on_frame #" << video_frame.frame_id()
                                  << ": format_id: " << core::utils::enum_to_string(video_frame.format().format_id())
                                  << ", fmt: " << video_frame.format().width()
                                  << "x" << video_frame.format().height()
                                  << "@" << video_frame.format().frame_rate()
                                  << ", ts: " << video_frame.timestamp()
                                  << ", kf: " << (video_frame.frame_type() == i_video_frame::frame_type_t::key_frame);

                        if (auto buffer = video_frame.buffers().get_buffer(0))
                        {
                            std::cout << ", size: " << buffer->size();
                        }

                        std::cout << std::endl;

                    }
                    else if (frame_message.frame().media_type() == media_type_t::audio)
                    {
                        const auto& audio_frame = static_cast<const i_audio_frame&>(frame_message.frame());
                        std::cout << "on_frame #" << audio_frame.frame_id()
                                  << ": format_id: " << core::utils::enum_to_string(audio_frame.format().format_id())
                                  << ", fmt: " << audio_frame.format().sample_rate()
                                  << "/" << audio_frame.format().channels()
                                  << ", ts: " << audio_frame.timestamp();

                        if (auto buffer = audio_frame.buffers().get_buffer(0))
                        {
                            std::cout << ", size: " << buffer->size();
                        }

                        std::cout << std::endl;
                    }
                }
                break;
                case message_category_t::event:
                {
                    const auto& event_message = static_cast<const i_message_event&>(message);
                    if (event_message.event().event_id == event_id_t::channel_state)
                    {
                        const auto& channel_state = static_cast<const event_channel_state_t&>(event_message.event());
                        std::cout << "device state: " << static_cast<std::uint32_t>(channel_state.state) << std::endl;
                    }
                }
                break;
                default:;
            }

            return true;
        };

        message_sink_impl sink(handler);

        input_device->source()->add_sink(&sink);
        input_device->control(channel_control_t::open());
        core::utils::sleep(durations::seconds(60));
        input_device->control(channel_control_t::close());
        input_device->source()->remove_sink(&sink);

    }


    return;
}

}

void tests()
{
    //test1();
    //test6();
    test12();
}

}
