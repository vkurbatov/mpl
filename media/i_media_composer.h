#ifndef MPL_I_MEDIA_COMPOSER_H
#define MPL_I_MEDIA_COMPOSER_H

#include "i_media_stream.h"

namespace mpl::media
{

class i_media_composer : public i_parametrizable
{
public:

    using u_ptr_t = std::unique_ptr<i_media_composer>;
    using s_ptr_t = std::shared_ptr<i_media_composer>;

    virtual ~i_media_composer() = default;

    virtual i_media_stream::s_ptr_t add_stream(const i_property& stream_property) = 0;
    virtual i_media_stream::s_ptr_t get_stream(stream_id_t stream_id) const = 0;

    virtual bool start() = 0;
    virtual bool stop() = 0;
    virtual bool is_started() const = 0;
};

}

#endif // MPL_I_MEDIA_COMPOSER_H
