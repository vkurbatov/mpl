#include <iostream>

#include "utils/ipc_manager_impl.h"
#include "core/i_message_event.h"
#include "core/i_message_source.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"
#include "utils/option_helper.h"
#include "utils/task_manager_impl.h"
#include "core/event_channel_state.h"

#include "media/libav_audio_converter_factory.h"
#include "media/libav_video_converter_factory.h"
#include "media/media_converter_factory_impl.h"
#include "media/libav_transcoder_factory.h"
#include "media/smart_transcoder_factory.h"
#include "media/ipc_output_device_factory.h"
#include "media/media_option_types.h"
#include "media/i_video_frame.h"

#include "media/audio_format_impl.h"
#include "media/video_format_impl.h"
#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"

#include "media/libav_input_device_factory.h"
#include "media/v4l2_device_factory.h"

#include "tools/ffmpeg/libav_base.h"
#include <string>
#include <thread>

int test()
{
    std::string ipc_manager_name = "mpl";
    std::size_t ipc_manager_size = 1024 * 1024 * 100;
    std::string ipc_channel_name = "test";
    std::size_t ipc_channel_size = 1024 * 1024 * 10;
    auto ipc_out_manager = mpl::ipc_manager_factory::get_instance().create_shared_data_manager(ipc_manager_name
                                                                                               , ipc_manager_size);
    mpl::i_sync_shared_data::s_ptr_t channel = ipc_out_manager->query_data(ipc_channel_name
                                                                            , ipc_channel_size);
    if (channel)
    {
        std::size_t notyfy_counter = 0;
        while(true)
        {
            channel->notify();
            std::cout << "notyfy: " << notyfy_counter << std::endl;
            notyfy_counter++;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    return 0;
}

int main()
{
    // return test();
    pt::ffmpeg::set_log_level(pt::ffmpeg::log_level_t::quiet);
    pt::ffmpeg::libav_register();
    std::string input_audio_url = "pulse://alsa_input.pci-0000_00_05.0.analog-stereo";
    std::string input_video_url = "/dev/video0";

    std::string ipc_manager_name = "mpl";
    std::size_t ipc_manager_size = 1024 * 1024 * 100;
    std::string ipc_channel_name = "test";
    std::size_t ipc_channel_size = 1024 * 1024 * 10;

    auto ipc_out_manager = mpl::ipc_manager_factory::get_instance().create_shared_data_manager(ipc_manager_name, ipc_manager_size);

    if (ipc_out_manager == nullptr)
    {
        return -1;
    }

    mpl::media::audio_format_impl audio_format(mpl::media::audio_format_id_t::aac
                                                , 48000
                                                , 2);

    mpl::media::video_format_impl video_format(mpl::media::video_format_id_t::h264
                                               , 1280
                                               , 720
                                               , 30);

    std::string encoder_options = "profile=baseline;preset=ultrafast;tune=zerolatency;cfr=30;g=60;keyint_min=30;max_delay=0;bf=0;threads=4";
    mpl::option_writer(video_format.options()).set(mpl::media::opt_codec_params, encoder_options);
    mpl::option_writer(audio_format.options()).set(mpl::media::opt_fmt_track_id, 0);
    mpl::option_writer(video_format.options()).set(mpl::media::opt_fmt_track_id, 1);


    auto libav_input_params = mpl::property_helper::create_object();
    if (libav_input_params)
    {
        mpl::property_writer writer(*libav_input_params);
        writer.set<std::string>("url", input_audio_url);
    }
    auto v4l2_input_params = mpl::property_helper::create_object();
    if (v4l2_input_params)
    {
        mpl::property_writer writer(*v4l2_input_params);
        writer.set<std::string>("url", input_video_url);
    }

    mpl::media::smart_transcoder_factory smart_factory(mpl::task_manager_factory::single_manager()
                                                        , mpl::media::libav_transcoder_factory::decoder_factory()
                                                        , mpl::media::libav_transcoder_factory::encoder_factory()
                                                        , mpl::media::media_converter_factory_impl::builtin_converter_factory());


    auto audio_transcoder = smart_factory.create_converter(*audio_format.get_params("format"));
    auto video_transcoder = smart_factory.create_converter(*video_format.get_params("format"));

    mpl::media::libav_input_device_factory input_device_factory;
    mpl::media::v4l2_device_factory v4l2_device_factory;

    mpl::media::ipc_output_device_factory ipc_output_device_factory(*ipc_out_manager);

    auto audio_device = input_device_factory.create_device(*libav_input_params);
    auto video_device = v4l2_device_factory.create_device(*v4l2_input_params);

    auto ipc_params = mpl::property_helper::create_object();
    if (ipc_params != nullptr)
    {
        mpl::property_writer writer(*ipc_params);
        writer.set("device_type", mpl::media::device_type_t::ipc_out);
        writer.set("channel_name", ipc_channel_name);
        writer.set("size", ipc_channel_size);
    }

    if (auto ipc_output_device = ipc_output_device_factory.create_device(*ipc_params))
    {
        mpl::message_sink_safe_impl sink(ipc_output_device->sink(0));

        audio_device->source(0)->add_sink(audio_transcoder.get());
        video_device->source(0)->add_sink(video_transcoder.get());

        audio_transcoder->set_sink(&sink);
        video_transcoder->set_sink(&sink);

        ipc_output_device->control(mpl::channel_control_t::open());
        audio_device->control(mpl::channel_control_t::open());
        video_device->control(mpl::channel_control_t::open());

        std::this_thread::sleep_for(std::chrono::minutes(360));
    }

    return 0;
}
