#include "app_engine_impl.h"
#include "app_config.h"
#include "i_app_module.h"

#include "net/net_module_types.h"
#include "media/media_module_types.h"
#include "app_module_types.h"

#include "event_message_factory_impl.h"
#include "command_message_factory_impl.h"

#include "utils/core_engine_impl.h"
#include "net/net_engine_impl.h"
#include "media/media_engine_impl.h"

namespace mpl::app
{

class app_engine_impl final : public i_app_engine
        , i_app_module
{
    app_config_t                    m_config;

    event_message_factory_impl      m_events;
    command_message_factory_impl    m_commands;

    i_core_engine::u_ptr_t          m_core_engine;
    net::i_net_engine::u_ptr_t      m_net_engine;
    media::i_media_engine::u_ptr_t  m_media_engine;

    // i_engine interface
public:

    using u_ptr_t = std::unique_ptr<app_engine_impl>;

    static u_ptr_t create(const app_config_t& config)
    {
        if (auto core_engine = core_engine_factory::get_instance().create_engine(config.core_config))
        {
            net::net_engine_factory net_factory(core_engine->core().task_manager()
                                                , core_engine->core().timer_manager());
            if (auto net_engine = net_factory.create_engine(config.net_config))
            {
                media::media_engine_factory media_factory(core_engine->core().task_manager()
                                                          , net_engine->net().transports());
                if (auto media_negine = media_factory.create_engine(config.media_config))
                {
                    return std::make_unique<app_engine_impl>(config
                                                             , std::move(core_engine)
                                                             , std::move(net_engine)
                                                             , std::move(media_negine));
                }
            }
        }

        return nullptr;
    }

    app_engine_impl(const app_config_t& config
                    , i_core_engine::u_ptr_t&& core_engine
                    , net::i_net_engine::u_ptr_t&& net_engine
                    , media::i_media_engine::u_ptr_t&& media_engine)
        : m_config(config)
        , m_core_engine(std::move(core_engine))
        , m_net_engine(std::move(net_engine))
        , m_media_engine(std::move(media_engine))
    {

    }

    ~app_engine_impl()
    {
        app_engine_impl::stop();
    }

    bool start() override
    {
        if (m_core_engine->start()
                && m_net_engine->start()
                && m_media_engine->start())
        {
            return true;
        }

        stop();
        return false;
    }

    bool stop() override
    {
        return m_media_engine->stop()
                & m_net_engine->stop()
                & m_core_engine->stop();
    }

    bool is_started() const override
    {
        return m_core_engine->is_started()
                && m_net_engine->is_started()
                && m_media_engine->is_started();
    }

    // i_app_engine interface
public:
    i_module *get_module(module_id_t module_id) override
    {
        switch(module_id)
        {
            case core_module_id:
                return &m_core_engine->core();
            break;
            case net::net_module_id:
                return &m_net_engine->net();
            break;
            case media::media_module_id:
                return &m_media_engine->media();
            break;
            case app_module_id:
                return this;
            break;
            default:;
        }

        return nullptr;
    }

    // i_module interface
public:
    module_id_t module_id() const override
    {
        return app_module_id;
    }

    // i_app_module interface
public:
    i_message_event_factory &events() override
    {
        return m_events;
    }

    i_message_command_factory &commands() override
    {
        return m_commands;
    }
};

app_engine_factory &app_engine_factory::get_instance()
{
    static app_engine_factory single_factory;
    return single_factory;
}

i_app_engine::u_ptr_t app_engine_factory::create_engine(const app_config_t &config)
{
    return app_engine_impl::create(config);
}

}
