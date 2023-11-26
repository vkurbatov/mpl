#ifndef MPL_I_BUFFER_COLLECTION_H
#define MPL_I_BUFFER_COLLECTION_H

#include "i_buffer.h"
#include <vector>
#include <cstdint>

namespace mpl
{

class i_buffer_collection
{
public:
    using u_ptr_t = std::unique_ptr<i_buffer_collection>;
    using s_ptr_t = std::shared_ptr<i_buffer_collection>;

    using index_t = std::int32_t;
    using index_list_t = std::vector<index_t>;

    virtual ~i_buffer_collection() = default;
    virtual const i_buffer* get_buffer(index_t index) const = 0;
    virtual index_list_t index_list() const = 0;
    virtual std::size_t count() const = 0;
};

}

#endif // MPL_I_BUFFER_COLLECTION_H
