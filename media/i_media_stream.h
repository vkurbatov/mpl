#ifndef MPL_I_MEDIA_STREAM_H
#define MPL_I_MEDIA_STREAM_H

#include "core/i_message_transceiver.h"
#include "core/i_parametrizable.h"
#include "media_types.h"
#include <vector>

namespace mpl::media
{

class i_media_stream : public i_parametrizable
        , public i_message_transceiver
{
public:
    using u_ptr_t = std::unique_ptr<i_media_stream>;
    using s_ptr_t = std::shared_ptr<i_media_stream>;
    using w_ptr_t = std::shared_ptr<i_media_stream>;
    using s_array_t = std::vector<s_ptr_t>;

    virtual ~i_media_stream() = default;

    virtual stream_id_t stream_id() const = 0;
    virtual std::string name() const = 0;
};

}

#endif // MPL_I_MEDIA_STREAM_H
