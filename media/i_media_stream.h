#ifndef MPL_I_MEDIA_STREAM_H
#define MPL_I_MEDIA_STREAM_H

#include "i_media_format.h"

namespace mpl::media
{

class i_media_stream
{
public:
    using u_ptr_t = std::unique_ptr<i_media_stream>;
    using s_ptr_t = std::shared_ptr<i_media_stream>;

    virtual const i_media_format& media_format() const = 0;
    virtual stream_id_t stream_id() const = 0;
};

}

#endif // MPL_I_MEDIA_STREAM_H
