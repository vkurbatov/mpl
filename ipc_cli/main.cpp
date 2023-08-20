#include <iostream>

#include "core/ipc/ipc_manager_impl.h"
#include "core/i_message_event.h"
#include "core/i_message_source.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"
#include "core/option_helper.h"

#include "media/libav_audio_converter_factory.h"
#include "media/libav_video_converter_factory.h"
#include "media/media_converter_factory_impl.h"
#include "media/libav_transcoder_factory.h"
#include "media/smart_transcoder_factory.h"
#include "media/ipc_input_device_factory.h"

#include "media/audio_format_impl.h"
#include "media/video_format_impl.h"
#include "media/audio_frame_impl.h"
#include "media/video_frame_impl.h"
#include "media/media_option_types.h"

#include "media/libav_output_device_factory.h"

#include "tools/ffmpeg/libav_base.h"
#include <string>
#include <thread>

int test()
{
    std::string ipc_manager_name = "mpl";
    std::string ipc_channel_name = "test";
    auto ipc_out_manager = mpl::ipc_manager_factory::get_instance().create_shared_data_manager(ipc_manager_name, 0);
    mpl::i_sync_shared_data::s_ptr_t channel = ipc_out_manager->query_data(ipc_channel_name
                                                                            , 0);

    std::size_t count = 10;
    if (channel)
    {
        while(count-- > 0)
        {
            if (channel->wait(mpl::durations::milliseconds(200)))
            {
                std::cout << "notyfy success" << std::endl;
            }
            else
            {
                std::cout << "notyfy timeout" << std::endl;
            }

        }
    }

    return 0;
}

int main()
{
    //return test();
    ffmpeg::libav_register();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::string output_url = "rtmp://127.0.0.1/cam1/stream";
    std::string ipc_manager_name = "mpl";
    std::string ipc_channel_name = "test";

    auto ipc_in_manager = mpl::ipc_manager_factory::get_instance().create_shared_data_manager(ipc_manager_name, 0);

    if (ipc_in_manager == nullptr)
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

    mpl::option_writer(audio_format.options()).set(mpl::media::opt_fmt_stream_id, 0);
    mpl::option_writer(video_format.options()).set(mpl::media::opt_fmt_stream_id, 1);

    auto libav_params = mpl::property_helper::create_object();
    if (libav_params)
    {
        mpl::property_writer writer(*libav_params);
        writer.set<std::string>("url", output_url);
        mpl::i_property::array_t streams;
        streams.emplace_back(audio_format.get_params());
        streams.emplace_back(video_format.get_params());
        writer.set("streams", streams);
    }

    mpl::media::libav_output_device_factory output_device_factory;
    mpl::media::ipc_input_device_factory ipc_input_device_factory(*ipc_in_manager);

    if (auto output_device = output_device_factory.create_device(*libav_params))
    {
        auto ipc_params = mpl::property_helper::create_object();
        if (ipc_params != nullptr)
        {
            mpl::property_writer writer(*ipc_params);
            writer.set("device_type", mpl::media::device_type_t::ipc_in);
            writer.set("channel_name", ipc_channel_name);
        }

        if (auto ipc_input_device = ipc_input_device_factory.create_device(*ipc_params))
        {

            ipc_input_device->source(0)->add_sink(output_device->sink(0));

            output_device->control(mpl::channel_control_t::open());
            ipc_input_device->control(mpl::channel_control_t::open());

            std::this_thread::sleep_for(std::chrono::minutes(360));
        }

    }

    return 0;
}

