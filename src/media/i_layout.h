#ifndef MPL_I_LAYOUT_H
#define MPL_I_LAYOUT_H

#include <memory>
#include "video_frame_types.h"

namespace mpl::media
{

class i_layout
{
public:
    using u_ptr_t = std::unique_ptr<i_layout>;
    using s_ptr_t = std::shared_ptr<i_layout>;

    virtual ~i_layout() = default;
    virtual std::size_t size() const = 0;
    virtual relative_frame_rect_t get_rect(std::size_t index) const = 0;
};

}

#endif // MPL_I_LAYOUT_H
