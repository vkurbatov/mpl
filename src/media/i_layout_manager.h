#ifndef MPL_MEDIA_I_LAYOUT_MANAGER_H
#define MPL_MEDIA_I_LAYOUT_MANAGER_H

#include "i_layout.h"

namespace mpl::media
{


class i_layout_manager
{
public:
    using u_ptr_t = std::unique_ptr<i_layout_manager>;
    using s_ptr_t = std::shared_ptr<i_layout_manager>;

    virtual ~i_layout_manager() = default;
    virtual const i_layout* query_layout(std::size_t streams) = 0;

};

}

#endif // MPL_MEDIA_I_LAYOUT_MANAGER_H
