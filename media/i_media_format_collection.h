#ifndef MPL_I_MEDIA_FORMAT_COLLECTION_H
#define MPL_I_MEDIA_FORMAT_COLLECTION_H

#include <memory>
#include "media_types.h"

namespace mpl::media
{

class i_media_format_factory;

class i_media_format_collection
{
public:
    using u_ptr_t = std::unique_ptr<i_media_format_collection>;
    using s_ptr_t = std::shared_ptr<i_media_format_collection>;

    virtual ~i_media_format_collection() = default;
    virtual i_media_format_factory* get_factory(media_type_t media_type) = 0;
};

}

#endif // MPL_I_MEDIA_FORMAT_COLLECTION_H
