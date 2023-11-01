#ifndef MPL_MEDIA_LAYOUT_MANAGER_MOSAIC_IMPL_H
#define MPL_MEDIA_LAYOUT_MANAGER_MOSAIC_IMPL_H

#include "media/i_layout_manager.h"

namespace mpl::media
{

class layout_manager_mosaic_impl : public i_layout_manager
{
public:

    static layout_manager_mosaic_impl& get_instance();

    layout_manager_mosaic_impl();

    // i_layout_manager interface
public:
    const i_layout* query_layout(std::size_t streams) override;
};

}

#endif // MPL_MEDIA_LAYOUT_MANAGER_MOSAIC_IMPL_H
