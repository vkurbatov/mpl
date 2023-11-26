#ifndef MPL_I_MEDIA_CONVERTER_FACTORY_H
#define MPL_I_MEDIA_CONVERTER_FACTORY_H

#include "i_media_converter.h"

namespace mpl::media
{

class i_media_converter_factory
{
public:
    using u_ptr_t = std::unique_ptr<i_media_converter_factory>;
    using s_ptr_t = std::shared_ptr<i_media_converter_factory>;

    virtual ~i_media_converter_factory() = default;
    virtual i_media_converter::u_ptr_t create_converter(const i_property& params) = 0;
};

}

#endif // MPL_I_MEDIA_CONVERTER_FACTORY_H
