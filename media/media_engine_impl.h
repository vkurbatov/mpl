#ifndef MPL_MEDIA_ENGINE_IMPL_H
#define MPL_MEDIA_ENGINE_IMPL_H

#include "core/i_task_manager.h"
#include "i_media_engine.h"

namespace mpl::net
{

class i_net_engine;

}

namespace mpl::media
{

struct media_engine_config_t;

class media_engine_impl : public i_media_engine
{
    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t             m_pimpl;

public:

    using u_ptr_t = std::unique_ptr<media_engine_impl>;
    using s_ptr_t = std::shared_ptr<media_engine_impl>;

    static media_engine_impl& get_instance();

    static u_ptr_t create(const media_engine_config_t& config
                          , i_task_manager& task_manager
                          , net::i_net_engine& net_engine);

    media_engine_impl(const media_engine_config_t& config
                      , i_task_manager& task_manager
                      , net::i_net_engine& net_engine);
    ~media_engine_impl();

    // i_engine interface
public:

    // i_media_engine interface
public:
    i_task_manager &task_manager() override;

    bool start() override;
    bool stop() override;
    bool is_started() const override;

    // i_media_engine interface
public:
    i_device_factory* device_factory(device_type_t device_type) override;
    i_media_format_factory* format_factory(media_type_t media_type) override;
    i_media_frame_builder::u_ptr_t create_frame_builder() override;
    i_layout_manager &layout_manager() override;
    i_media_converter_factory* converter_factory(media_converter_type_t type) override;
    i_media_composer_factory::u_ptr_t create_composer_factory(i_layout_manager &layout_manager) override;
};

}

#endif // MPL_MEDIA_ENGINE_IMPL_H
