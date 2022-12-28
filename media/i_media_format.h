#ifndef MPL_I_MEDIA_FORMAT_H
#define MPL_I_MEDIA_FORMAT_H

#include "media_types.h"
#include "i_buffer.h"
#include "i_property.h"
#include "i_option.h"

#include <memory>

namespace mpl
{

class i_media_format
{
public:
    using u_ptr_t = std::unique_ptr<i_media_format>;
    using s_ptr_t = std::shared_ptr<i_media_format>;
    virtual ~i_media_format() = default;
    virtual media_type_t media_type() const = 0;
    virtual bool is_encoded() const = 0;
    virtual bool is_convertable() const = 0;
    virtual u_ptr_t clone() const = 0;
    virtual bool is_equal(const i_media_format& other) const = 0;
    virtual bool is_compatible(const i_media_format& other) const = 0;
    virtual const i_option& options() const = 0;
    virtual bool is_valid() const = 0;
};

}

#endif // MPL_I_MEDIA_FORMAT_H
