#include <iostream>

#include "core/ipc/ipc_manager_impl.h"
#include "core/i_message_event.h"
#include "core/i_message_source.h"
#include "core/message_sink_impl.h"
#include "core/property_writer.h"

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

#include "media/libav_output_device_factory.h"

#include "tools/ffmpeg/libav_base.h"
#include <string>
#include <thread>

int main()
{
    ffmpeg::libav_register();
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

    mpl::media::smart_transcoder_factory smart_factory(mpl::media::libav_transcoder_factory::decoder_factory()
                                                        , mpl::media::libav_transcoder_factory::encoder_factory()
                                                        , mpl::media::media_converter_factory_impl::builtin_converter_factory());

    mpl::media::libav_output_device_factory output_device_factory;
    mpl::media::ipc_input_device_factory ipc_input_device_factory(*ipc_in_manager);

    if (auto output_device = output_device_factory.create_device(*libav_params))
    {

        auto handler = [&](const mpl::i_message& message)
        {
            switch(message.category())
            {
                case mpl::message_category_t::frame:
                    return output_device->sink()->send_message(message);
                break;
            }

            return false;
        };

        mpl::message_sink_impl sink(handler);

        auto audio_transcoder = smart_factory.create_converter(*audio_format.get_params("format"));
        auto video_transcoder = smart_factory.create_converter(*video_format.get_params("format"));

        audio_transcoder->set_sink(&sink);
        video_transcoder->set_sink(&sink);

        auto ipc_params = mpl::property_helper::create_object();
        if (ipc_params != nullptr)
        {
            mpl::property_writer writer(*ipc_params);
            writer.set("device_type", mpl::media::device_type_t::ipc_in);
            writer.set("channel_name", ipc_channel_name);
        }

        if (auto ipc_input_device = ipc_input_device_factory.create_device(*ipc_params))
        {
            ipc_input_device->source()->add_sink(audio_transcoder.get());
            ipc_input_device->source()->add_sink(video_transcoder.get());

            output_device->control(mpl::channel_control_t::open());
            ipc_input_device->control(mpl::channel_control_t::open());

            std::this_thread::sleep_for(std::chrono::minutes(5));
        }

    }

    return 0;
}

