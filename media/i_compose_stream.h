#ifndef MPL_MEDIA_I_COMPOSE_STREAM_H
#define MPL_MEDIA_I_COMPOSE_STREAM_H

#include "i_media_stream.h"

namespace mpl::media
{

class i_compose_stream : public i_media_stream
{
public:
    using u_ptr_t = std::unique_ptr<i_compose_stream>;
    using s_ptr_t = std::shared_ptr<i_compose_stream>;
    using w_ptr_t = std::weak_ptr<i_compose_stream>;

    virtual ~i_compose_stream() = default;


};

}

#endif // MPL_MEDIA_I_COMPOSE_STREAM_H
