#ifndef MPL_SMART_BUFFER_H
#define MPL_SMART_BUFFER_H

#include "common_types.h"
#include "i_dynamic_buffer.h"

namespace mpl
{

class smart_buffer : public i_dynamic_buffer
{
    enum class storage_type_t
    {
        undefined = -1,
        ref_const_data,
        ref_const_array,
        ref_const_data_pointer,
        shared_data_pointer,
        stored_data
    };

    storage_type_t                      m_storage_type;
    union
    {
        const void*                     m_ref_const_data;
        const raw_array_t*              m_ref_const_array;
        const i_data_object*            m_ref_const_data_pointer;
    };

    std::size_t                         m_ref_size;
    i_data_object::s_ptr_t              m_storage_context;
    smart_buffer(const i_data_object::s_ptr_t& storage_context
                 , bool storage);
public:
    using u_ptr_t = std::unique_ptr<smart_buffer>;
    using s_ptr_t = std::shared_ptr<smart_buffer>;
    using array_t = std::vector<smart_buffer>;


    static u_ptr_t create();
    static u_ptr_t create(const void* const_data
                           , std::size_t size
                           , bool copy = false);
    static u_ptr_t create(const raw_array_t* const_array);
    static u_ptr_t create(raw_array_t&& array);
    static u_ptr_t create(const i_data_object* data_object_ptr);
    static u_ptr_t create(const i_data_object::s_ptr_t& data_object);

    smart_buffer();
    smart_buffer(const void* const_data
                   , std::size_t size
                   , bool copy = false);
    smart_buffer(const raw_array_t* const_array);
    smart_buffer(raw_array_t&& array);
    smart_buffer(const i_data_object* data_object_ptr);
    smart_buffer(const i_data_object::s_ptr_t& data_object);
    smart_buffer(const smart_buffer& other) = default;
    smart_buffer(smart_buffer&& other);


    smart_buffer& operator=(const smart_buffer& other) = default;
    smart_buffer& operator=(smart_buffer&& other);

    smart_buffer& assign(const void* const_data
                          , std::size_t size
                          , bool copy = false);
    smart_buffer& assign(const raw_array_t* const_array);
    smart_buffer& assign(raw_array_t&& array);
    smart_buffer& assign(const i_data_object* data_object_ptr);
    smart_buffer& assign(const i_data_object::s_ptr_t& data_object);

    bool operator == (const smart_buffer& other) const;
    bool operator != (const smart_buffer& other) const;

    bool is_empty() const;
    bool is_valid() const;
    void reset();

    void make_store();
    smart_buffer fork() const;

    const std::uint8_t& operator[](std::int32_t index) const;

    // i_data_container interface
public:
    const void *data() const override;
    void *map() override;
    std::size_t size() const override;
    i_buffer::u_ptr_t clone() const override;
    std::size_t refs() const override;
    void make_shared() override;
    // i_dynamic_buffer interface
public:
    bool append_data(const void *data, std::size_t size) override;
    void resize(std::size_t new_size) override;
    void clear() override;
private:
    bool is_shared() const;
};

}

#endif // MPL_SMART_BUFFER_H
