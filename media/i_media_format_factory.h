#ifndef MPL_MEDIA_I_MEDIA_FORMAT_FACTORY_H
#define MPL_MEDIA_I_MEDIA_FORMAT_FACTORY_H

#include "i_media_format.h"

namespace mpl::media
{
class i_media_format_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_media_format_factory>;
    using s_ptr_t = std::shared_ptr<i_media_format_factory>;

    virtual i_media_format::u_ptr_t create_format(const i_property& format_params) = 0;
};

}

#endif // MPL_MEDIAI_MEDIA_FORMAT_FACTORY_H
