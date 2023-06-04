#ifndef MPL_I_MEDIA_STREAM_H
#define MPL_I_MEDIA_STREAM_H

#include "core/i_message_sink.h"
#include "core/i_message_source.h"
#include "core/i_parametrizable.h"
#include "i_media_format.h"

namespace mpl::media
{

class i_media_stream : public i_parametrizable
{
public:
    using u_ptr_t = std::unique_ptr<i_media_stream>;
    using s_ptr_t = std::shared_ptr<i_media_stream>;
    using w_ptr_t = std::shared_ptr<i_media_stream>;

    virtual ~i_media_stream() = default;

    virtual stream_id_t stream_id() const = 0;

    virtual i_message_sink* sink() = 0;
    virtual i_message_source* source() = 0;


};

}

#endif // MPL_I_MEDIA_STREAM_H
