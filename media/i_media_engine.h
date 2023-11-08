#ifndef MPL_I_MEDIA_ENGINE_H
#define MPL_I_MEDIA_ENGINE_H

#include "core/i_engine.h"
#include "i_device_factory.h"
#include "i_media_frame_builder.h"
#include "i_media_converter_factory.h"
#include "i_media_composer_factory.h"
#include "i_layout_manager.h"
#include "media_types.h"


namespace mpl
{

class i_task_manager;

namespace media
{

class i_media_format_factory;
class i_device_factory_collection;

class i_media_engine : public i_engine
{
public:

    enum class media_converter_type_t
    {
        undefined = 0,
        converter,
        encoder,
        decoder,
        smart
    };

    using u_ptr_t = std::unique_ptr<i_media_engine>;
    using s_ptr_t = std::shared_ptr<i_media_engine>;

    virtual i_layout_manager& layout_manager() = 0;
    virtual i_device_factory_collection& device_collection() = 0;
    virtual i_media_format_factory* format_factory(media_type_t media_type) = 0;
    virtual i_media_frame_builder::u_ptr_t create_frame_builder() = 0;
    virtual i_media_converter_factory* converter_factory(media_converter_type_t type) = 0;
    virtual i_media_composer_factory::u_ptr_t create_composer_factory(i_layout_manager& layout_manager) = 0;

};

}

}

#endif // MPL_I_MEDIA_ENGINE_H
