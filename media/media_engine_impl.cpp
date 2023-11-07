#include "media_engine_impl.h"
#include "media_engine_config.h"
#include "media_frame_builder_impl.h"
#include "media_format_factory_impl.h"

#include "tools/ffmpeg/libav_base.h"

#include "utils/ipc_manager_impl.h"

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

#include "net/i_net_engine.h"

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


struct media_engine_impl::pimpl_t
{
    using u_ptr_t = media_engine_impl::pimpl_ptr_t;

    media_engine_config_t               m_config;
    i_task_manager&                     m_task_manager;
    net::i_net_engine&                  m_net_engine;

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

    layout_manager_mosaic_impl          m_layout_manager;

    libav_audio_converter_factory       m_audio_converter_factory;
    libav_video_converter_factory       m_video_converter_factory;
    media_converter_factory_impl        m_media_converter_factory;
    libav_transcoder_factory            m_decoder_factory;
    libav_transcoder_factory            m_encoder_factory;
    smart_transcoder_factory            m_smart_transcoder_factory;

    std::atomic_bool                    m_start;

    static u_ptr_t create(const media_engine_config_t& config
                          , i_task_manager& task_manager
                          , net::i_net_engine& net_engine)
    {
        detail::check_and_init_libav();
        return std::make_unique<pimpl_t>(config
                                         , task_manager
                                         , net_engine);
    }

    static visca_device_factory::u_ptr_t create_visca_device(net::i_net_engine& net_engine)
    {
        if (auto serial_factory = net_engine.transport_factory(net::transport_id_t::serial))
        {
            return visca_device_factory::create(*serial_factory);
        }

        return nullptr;
    }

    pimpl_t(const media_engine_config_t& config
            , i_task_manager& task_manager
            , net::i_net_engine& net_engine)
        : m_config(config)
        , m_task_manager(task_manager)
        , m_net_engine(net_engine)
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
        , m_visca_factory(create_visca_device(m_net_engine))
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

    }

    ~pimpl_t()
    {
        stop();
    }

    inline bool start()
    {
        bool flag  = false;
        if (m_start.compare_exchange_strong(flag, true))
        {
            if (m_task_manager.is_started())
            {
                return true;
            }

            m_start.store(false, std::memory_order_release);
        }

        return false;
    }

    inline bool stop()
    {
        bool flag = true;
        return m_start.compare_exchange_strong(flag, false);
    }

    inline bool is_started() const
    {
        return m_start.load(std::memory_order_acquire)
                && m_task_manager.is_started();
    }

    inline i_device_factory* device_factory(device_type_t device_type)
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

    inline i_media_format_factory *format_factory(media_type_t media_type)
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

    inline i_media_frame_builder::u_ptr_t create_frame_builder()
    {
        return media_frame_builder_impl::create();
    }

    inline i_layout_manager &layout_manager()
    {
        return m_layout_manager;
    }

    inline i_media_converter_factory* converter_factory(i_media_engine::media_converter_type_t type)
    {
        switch(type)
        {
            case i_media_engine::media_converter_type_t::converter:
                return &m_media_converter_factory;
            break;
            case i_media_engine::media_converter_type_t::decoder:
                return &m_decoder_factory;
            break;
            case i_media_engine::media_converter_type_t::encoder:
                return &m_encoder_factory;
            break;
            case i_media_engine::media_converter_type_t::smart:
                return &m_smart_transcoder_factory;
            break;
            default:;
        }

        return nullptr;
    }

    inline i_media_composer_factory::u_ptr_t create_composer_factory(i_layout_manager &layout_manager)
    {
        return media_composer_factory_impl::create(m_smart_transcoder_factory
                                                   , layout_manager);

    }
};

media_engine_impl::u_ptr_t media_engine_impl::create(const media_engine_config_t &config
                                                     , i_task_manager &task_manager
                                                     , net::i_net_engine& net_engine)
{
    return std::make_unique<media_engine_impl>(config
                                               , task_manager
                                               , net_engine);
}

media_engine_impl::media_engine_impl(const media_engine_config_t& config
                                     , i_task_manager &task_manager
                                     , net::i_net_engine& net_engine)
    : m_pimpl(pimpl_t::create(config
                              , task_manager
                              , net_engine))
{

}

i_task_manager &media_engine_impl::task_manager()
{
    return m_pimpl->m_task_manager;
}

bool media_engine_impl::start()
{
    return m_pimpl->start();
}

bool media_engine_impl::stop()
{
    return m_pimpl->stop();
}

bool media_engine_impl::is_started() const
{
    return m_pimpl->is_started();
}

i_device_factory* media_engine_impl::device_factory(device_type_t device_type)
{
    return m_pimpl->device_factory(device_type);
}

i_media_format_factory* media_engine_impl::format_factory(media_type_t media_type)
{
    return m_pimpl->format_factory(media_type);
}

i_media_frame_builder::u_ptr_t media_engine_impl::create_frame_builder()
{
    return m_pimpl->create_frame_builder();
}

i_layout_manager &media_engine_impl::layout_manager()
{
    return m_pimpl->layout_manager();
}

i_media_converter_factory* media_engine_impl::converter_factory(media_converter_type_t type)
{
    return m_pimpl->converter_factory(type);
}

i_media_composer_factory::u_ptr_t media_engine_impl::create_composer_factory(i_layout_manager &layout_manager)
{
    return m_pimpl->create_composer_factory(layout_manager);
}


}
