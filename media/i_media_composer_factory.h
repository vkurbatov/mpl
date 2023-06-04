#ifndef MPL_I_MEDIA_COMPOSER_FACTORY_H
#define MPL_I_MEDIA_COMPOSER_FACTORY_H

#include "i_media_composer.h"

namespace mpl::media
{

class i_media_composer_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_media_composer>;
    using s_ptr_t = std::unique_ptr<i_media_composer>;

    virtual ~i_media_composer_factory() = default;
    virtual i_media_composer::u_ptr_t create_composer(const i_property& params) = 0;
};

}

#endif // MPL_I_MEDIA_COMPOSER_FACTORY_H
