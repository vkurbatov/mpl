#ifndef MPL_I_MEDIA_ENGINE_H
#define MPL_I_MEDIA_ENGINE_H

#include "core/i_engine.h"


namespace mpl
{

class i_task_manager;

namespace media
{

class i_media_engine : public i_engine
{
public:
    using u_ptr_t = std::unique_ptr<i_media_engine>;
    using s_ptr_t = std::shared_ptr<i_media_engine>;

    virtual i_task_manager& task_manager() = 0;

    // ???
};

}

}

#endif // MPL_I_MEDIA_ENGINE_H
