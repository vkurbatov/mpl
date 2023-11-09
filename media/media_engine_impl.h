#ifndef MPL_MEDIA_ENGINE_IMPL_H
#define MPL_MEDIA_ENGINE_IMPL_H

#include "core/i_task_manager.h"
#include "i_media_engine.h"

namespace mpl::net
{

class i_transport_collection;

}

namespace mpl::media
{

struct media_engine_config_t;

class media_engine_factory
{
    i_task_manager&                 m_task_manager;
    net::i_transport_collection&    m_transports;

public:

    media_engine_factory(i_task_manager& task_manager
                         , net::i_transport_collection& transports);

    i_media_engine::u_ptr_t create_engine(const media_engine_config_t& config);
};

}

#endif // MPL_MEDIA_ENGINE_IMPL_H
