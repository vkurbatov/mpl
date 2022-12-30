#ifndef MPL_SMART_BUFFER_COLLECTION_H
#define MPL_SMART_BUFFER_COLLECTION_H

#include "smart_buffer.h"
#include "i_buffer_collection.h"

#include <unordered_map>

namespace mpl
{

class smart_buffer_collection : public i_buffer_collection
{
    using smart_buffer_map_t = std::unordered_map<std::int32_t, smart_buffer>;
    smart_buffer_map_t      m_buffers;
public:
    smart_buffer_collection();

    void set_buffer(std::int32_t index, const smart_buffer& buffer);
    void set_buffer(std::int32_t index, smart_buffer&& buffer);
    bool remove_buffer(std::int32_t index);

    smart_buffer* get_smart_buffer(std::int32_t index);
    const smart_buffer* get_smart_buffer(std::int32_t index) const;

    void make_shared();
    smart_buffer_collection fork() const;

    void assign(const i_buffer_collection& other);

    // i_buffer_collection interface
public:
    const i_buffer *get_buffer(index_t index) const override;
    index_list_t index_list() const override;
    std::size_t count() const override;


};

}

#endif // MPL_SMART_BUFFER_COLLECTION_H
