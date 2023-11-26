#ifndef I_MEDIA_TRACK_H
#define I_MEDIA_TRACK_H

#include "core/i_parametrizable.h"
#include "i_media_format.h"

namespace mpl::media
{

class i_media_stream;

class i_media_track : public i_parametrizable
{
public:
    using u_ptr_t = std::unique_ptr<i_media_track>;
    using s_ptr_t = std::shared_ptr<i_media_track>;
    using w_ptr_t = std::shared_ptr<i_media_track>;

    virtual const i_media_stream& media_stream() const = 0;
    virtual track_id_t track_id() const = 0;
    virtual std::string name() const = 0;
    virtual bool is_enabled() const = 0;
    virtual bool set_enabled(bool enabled) = 0;

    virtual const i_media_format& format() const = 0;
};

}

#endif // I_MEDIA_TRACK_H
