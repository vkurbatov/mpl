#include "media_engine_impl.h"
#include "media_engine_config.h"
#include "media_frame_builder_impl.h"
#include "media_format_factory_impl.h"
#include "media_module_types.h"
#include "i_media_converter_collection.h"

#include "tools/ffmpeg/libav_base.h"

#include "utils/ipc_manager_impl.h"
#include "i_device_factory_collection.h"
#include "i_media_format_collection.h"

#include "apm/apm_device_factory.h"
#include "ipc/ipc_input_device_factory.h"
#include "ipc/ipc_output_device_factory.h"
#include "libav/libav_input_device_factory.h"
#include "libav/libav_output_device_factory.h"
#include "v4l2/v4l2_device_factory.h"
#include "vnc/vnc_device_factory.h"
#include "visca/visca_device_factory.h"

#include "mcu/layout_manager_mosaic_impl.h"
#include "mcu/media_composer_factory_impl.h"
#include "libav/libav_audio_converter_factory.h"
#include "libav/libav_video_converter_factory.h"
#include "libav/libav_transcoder_factory.h"
#include "media_converter_factory_impl.h"
#include "smart_transcoder_factory.h"

#include "net/i_transport_collection.h"

#include "log/log_tools.h"


#include <atomic>

namespace mpl::media
{

namespace detail
{

inline void check_and_init_libav()
{
    if (!pt::ffmpeg::is_registered())
    {
        pt::ffmpeg::libav_register();
    }
}

}


struct media_engine_impl final : public i_media_engine
        , i_media_module
        , i_device_collection
        , i_media_converter_collection
        , i_media_format_collection
{
    using u_ptr_t = std::unique_ptr<media_engine_impl>;

    media_engine_config_t               m_config;
    i_task_manager&                     m_task_manager;
    net::i_transport_collection&        m_transports;

    media_format_factory_impl           m_audio_format_factory;
    media_format_factory_impl           m_video_format_factory;

    i_shared_data_manager::u_ptr_t      m_ipc_manager;

    ipc_input_device_factory::u_ptr_t   m_ipc_input_device_factory;
    ipc_output_device_factory::u_ptr_t  m_ipc_output_device_factory;
    apm_device_factory                  m_apm_device_factory;
    libav_input_device_factory          m_libav_input_device_factory;
    libav_output_device_factory         m_libav_output_device_factory;
    v4l2_device_factory                 m_v4l2_input_device_factory;
    vnc_device_factory                  m_vnc_device_factory;
    visca_device_factory::u_ptr_t       m_visca_factory;

    libav_audio_converter_factory       m_audio_converter_factory;
    libav_video_converter_factory       m_video_converter_factory;
    media_converter_factory_impl        m_media_converter_factory;
    libav_transcoder_factory            m_decoder_factory;
    libav_transcoder_factory            m_encoder_factory;
    smart_transcoder_factory            m_smart_transcoder_factory;

    std::atomic_bool                    m_start;

    static visca_device_factory::u_ptr_t create_visca_device(net::i_transport_collection& transports)
    {
        if (auto serial_factory = transports.get_factory(net::transport_id_t::serial))
        {
            return visca_device_factory::create(*serial_factory);
        }

        return nullptr;
    }

    static u_ptr_t create(const media_engine_config_t& config
                          , i_task_manager& task_manager
                          , net::i_transport_collection& transports)
    {
        detail::check_and_init_libav();
        return std::make_unique<media_engine_impl>(config
                                                  , task_manager
                                                  , transports);
    }

    media_engine_impl(const media_engine_config_t& config
            , i_task_manager& task_manager
            , net::i_transport_collection& transports)
        : m_config(config)
        , m_task_manager(task_manager)
        , m_transports(transports)
        , m_audio_format_factory(media_type_t::audio)
        , m_video_format_factory(media_type_t::video)
        , m_ipc_manager(ipc_manager_factory::get_instance().create_shared_data_manager(m_config.ipc_name
                                                                                       , m_config.ipc_size))
        , m_ipc_input_device_factory(m_ipc_manager != nullptr
                                     ? ipc_input_device_factory::create(*m_ipc_manager)
                                     : nullptr)
        , m_ipc_output_device_factory(m_ipc_manager != nullptr
                                     ? ipc_output_device_factory::create(*m_ipc_manager)
                                     : nullptr)
        , m_visca_factory(create_visca_device(transports))
        , m_media_converter_factory(m_audio_converter_factory
                                    , m_video_converter_factory)
        , m_decoder_factory(false)
        , m_encoder_factory(true)
        , m_smart_transcoder_factory(m_task_manager
                                     , m_decoder_factory
                                     , m_encoder_factory
                                     , m_media_converter_factory)
        , m_start(false)
    {
        mpl_log_info("Media engine #", this, ": init");
    }

    ~media_engine_impl()
    {
        mpl_log_info("Media engine #", this, ": destruction");
        media_engine_impl::stop();
    }

    bool start()override
    {
        bool flag  = false;
        if (m_start.compare_exchange_strong(flag, true))
        {            
            if (m_task_manager.is_started())
            {
                mpl_log_info("Media engine #", this, ": start success");
                return true;
            }

            mpl_log_error("Media engine #", this, ": start error: task manager not started");

            m_start.store(false, std::memory_order_release);
        }

        return false;
    }

    bool stop() override
    {
        bool flag = true;
        if (m_start.compare_exchange_strong(flag, false))
        {
            mpl_log_info("Media engine #", this, ": stop success");
            return true;
        }

        return false;
    }

    bool is_started() const override
    {
        return m_start.load(std::memory_order_acquire)
                && m_task_manager.is_started();
    }

    i_device_collection& devices() override
    {
        return *this;
    }

    i_media_converter_collection& converters() override
    {
        return *this;
    }

    i_media_format_collection& formats() override
    {
        return *this;
    }

    i_media_frame_builder::u_ptr_t create_frame_builder() override
    {
        return media_frame_builder_impl::create();
    }

    i_media_composer_factory::u_ptr_t create_composer_factory(i_layout_manager &layout_manager) override
    {
        return media_composer_factory_impl::create(m_smart_transcoder_factory
                                                   , layout_manager);

    }

    i_device_factory* get_factory(device_type_t device_type) override
    {
        switch(device_type)
        {
            case device_type_t::apm:
                return &m_apm_device_factory;
            break;
            case device_type_t::ipc_in:
                return m_ipc_input_device_factory.get();
            break;
            case device_type_t::ipc_out:
                return m_ipc_output_device_factory.get();
            break;
            case device_type_t::libav_in:
                return &m_libav_input_device_factory;
            break;
            case device_type_t::libav_out:
                return &m_libav_output_device_factory;
            break;
            case device_type_t::v4l2_in:
                return &m_v4l2_input_device_factory;
            break;
            case device_type_t::vnc:
                return &m_vnc_device_factory;
            break;
            case device_type_t::visca:
               return m_visca_factory.get();
            break;
            default:;
        }

        return nullptr;
    }

    i_media_converter_factory* get_factory(media_converter_type_t type) override
    {
        switch(type)
        {
            case media_converter_type_t::converter:
                return &m_media_converter_factory;
            break;
            case media_converter_type_t::decoder:
                return &m_decoder_factory;
            break;
            case media_converter_type_t::encoder:
                return &m_encoder_factory;
            break;
            case media_converter_type_t::smart:
                return &m_smart_transcoder_factory;
            break;
            default:;
        }


        return nullptr;
    }

    // i_media_format_collection interface
public:
    i_media_format_factory *get_factory(media_type_t media_type) override
    {
        switch(media_type)
        {
            case media_type_t::audio:
                return &m_audio_format_factory;
            break;
            case media_type_t::video:
                return &m_video_format_factory;
            break;
            default:;
        }

        return nullptr;
    }

    // i_module interface
public:
    module_id_t module_id() const override
    {
        return media_module_id;
    }

    // i_media_engine interface
public:
    i_media_module &media() override
    {
        return *this;
    }
};


media_engine_factory::media_engine_factory(i_task_manager &task_manager
                                           , net::i_transport_collection &transports)
    : m_task_manager(task_manager)
    , m_transports(transports)
{

}

i_media_engine::u_ptr_t media_engine_factory::create_engine(const media_engine_config_t &config)
{
    return media_engine_impl::create(config
                                     , m_task_manager
                                     , m_transports);
}


}
