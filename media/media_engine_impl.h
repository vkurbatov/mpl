#ifndef MPL_MEDIA_ENGINE_IMPL_H
#define MPL_MEDIA_ENGINE_IMPL_H

#include "core/i_task_manager.h"
#include "i_media_engine.h"

namespace mpl::media
{

class media_engine_impl : public i_media_engine
{
public:
    struct config_t
    {

    };

private:

    struct pimpl_t;
    using pimpl_ptr_t = std::unique_ptr<pimpl_t>;

    pimpl_ptr_t             m_pimpl;

public:

    using u_ptr_t = std::unique_ptr<media_engine_impl>;
    using s_ptr_t = std::shared_ptr<media_engine_impl>;

    static media_engine_impl& get_instance();

    static u_ptr_t create(const config_t& config
                          , i_task_manager& task_manager);

    media_engine_impl(const config_t& config
                    , i_task_manager& task_manager);
    ~media_engine_impl();

    // i_engine interface
public:

    // i_media_engine interface
public:
    i_task_manager &task_manager() override;

    bool start() override;
    bool stop() override;
    bool is_started() const override;

};

}

#endif // MPL_MEDIA_ENGINE_IMPL_H
