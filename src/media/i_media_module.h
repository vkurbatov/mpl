#ifndef MPL_I_MEDIA_MODULE_H
#define MPL_I_MEDIA_MODULE_H

#include "i_media_frame_builder.h"
#include "i_media_composer_factory.h"
#include "core/i_module.h"

namespace mpl::media
{

class i_layout_manager;
class i_media_format_factory;
class i_device_collection;
class i_media_converter_collection;
class i_media_format_collection;

class i_media_module : public i_module
{
public:
    using u_ptr_t = std::unique_ptr<i_media_module>;
    using s_ptr_t = std::shared_ptr<i_media_module>;

    virtual i_device_collection& devices() = 0;
    virtual i_media_format_collection& formats() = 0;
    virtual i_media_converter_collection& converters() = 0;
    virtual i_media_frame_builder::u_ptr_t create_frame_builder() = 0;
    virtual i_media_composer_factory::u_ptr_t create_composer_factory(i_layout_manager& layout_manager) = 0;
};

}

#endif // MPL_I_MEDIA_MODULE_H
