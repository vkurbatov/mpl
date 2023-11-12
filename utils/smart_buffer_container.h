#ifndef MPL_UTILS_SMART_BUFFER_CONTAINER_H
#define MPL_UTILS_SMART_BUFFER_CONTAINER_H

#include "smart_buffer.h"

namespace mpl::utils
{

class smart_buffer_container
{
protected:
    smart_buffer    m_buffer;

public:
    smart_buffer_container(const smart_buffer& buffer);
    smart_buffer_container(smart_buffer&& buffer);

    smart_buffer& get_buffer();
    const smart_buffer& get_buffer() const;

    void set_buffer(const smart_buffer& buffer);
    void set_buffer(smart_buffer&& buffer);

    smart_buffer release_buffer();
};

}


#endif // MPL_UTILS_SMART_BUFFER_CONTAINER_H
