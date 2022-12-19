#include "smart_buffer.h"
#include "utils.h"

#include <cstring>

namespace mpl
{

struct storage_context_t : public i_data_object
{
    using s_ptr_t = std::shared_ptr<storage_context_t>;
    raw_array_t         m_data;

    static s_ptr_t create(raw_array_t&& array)
    {
        return std::make_shared<storage_context_t>(std::move(array));
    }

    storage_context_t(raw_array_t&& array)
        : m_data(std::move(array))
    {

    }

    inline const void* data() const override
    {
        return m_data.data();
    }

    inline std::size_t size() const override
    {
        return m_data.size();
    }

    inline void* map()
    {
        return m_data.data();
    }

    inline void append(const void* data, std::size_t size)
    {
        m_data.insert(m_data.end()
                      , static_cast<const raw_array_t::value_type*>(data)
                      , static_cast<const raw_array_t::value_type*>(data) + size);
    }

    inline void resize(std::size_t new_size)
    {
        m_data.resize(new_size);
    }

    inline void clear()
    {
        m_data.clear();
    }

};

smart_buffer::smart_buffer(i_data_object::s_ptr_t storage_context
                           , bool storage)
    : m_storage_context(storage_context)
{
    m_storage_type = storage
            ? storage_type_t::stored_data
            : storage_type_t::shared_data_pointer;
}


smart_buffer::u_ptr_t smart_buffer::create()
{
    return std::make_unique<smart_buffer>();
}

smart_buffer::u_ptr_t smart_buffer::create(const void *const_data
                                                 , std::size_t size
                                                 , bool copy)
{
    return std::make_unique<smart_buffer>(const_data
                                            , size
                                            , copy);
}


smart_buffer::u_ptr_t smart_buffer::create(const raw_array_t *const_array)
{
    return std::make_unique<smart_buffer>(const_array);
}


smart_buffer::u_ptr_t smart_buffer::create(raw_array_t &&array)
{
    return std::make_unique<smart_buffer>(std::move(array));
}

smart_buffer::u_ptr_t smart_buffer::create(const i_data_object *data_object_ptr)
{
    return std::make_unique<smart_buffer>(data_object_ptr);
}

smart_buffer::u_ptr_t smart_buffer::create(i_data_object::s_ptr_t data_object)
{
    return std::make_unique<smart_buffer>(std::move(data_object));
}

smart_buffer::smart_buffer()
    : m_storage_type(storage_type_t::undefined)
{
    reset();
}

smart_buffer::smart_buffer(const void *const_data
                               , std::size_t size
                               , bool copy)
    : smart_buffer()
{
    assign(const_data
           , size
           , copy);
}


smart_buffer::smart_buffer(const raw_array_t *const_array)
    : smart_buffer()
{
    assign(const_array);
}

smart_buffer::smart_buffer(raw_array_t &&array)
    : smart_buffer()
{
    assign(std::move(array));
}

smart_buffer::smart_buffer(const i_data_object *data_object_ptr)
    : smart_buffer()
{
    assign(data_object_ptr);
}

smart_buffer::smart_buffer(i_data_object::s_ptr_t data_object)
    : smart_buffer()
{
    assign(std::move(data_object));
}


smart_buffer::smart_buffer(smart_buffer &&other)
    : smart_buffer()
{
    operator = (std::move(other));
}

smart_buffer &smart_buffer::operator=(smart_buffer &&other)
{
    m_storage_type = std::move(other.m_storage_type);
    m_ref_const_data = std::move(other.m_ref_const_array);
    m_ref_size = std::move(other.m_ref_size);
    m_storage_context = std::move(other.m_storage_context);
    other.reset();
    return *this;
}

smart_buffer &smart_buffer::assign(const void *const_data, std::size_t size, bool copy)
{
    reset();
    if (copy || const_data == nullptr)
    {
        assign(utils::create_raw_array(const_data, size));
    }
    else
    {
        m_ref_const_data = const_data;
        m_ref_size = size;
        m_storage_type = storage_type_t::ref_const_data;
    }

    return *this;
}

smart_buffer &smart_buffer::assign(const raw_array_t *const_array)
{
    reset();

    m_ref_const_array = const_array;
    m_storage_type = storage_type_t::ref_const_data;

    return *this;
}

smart_buffer &smart_buffer::assign(raw_array_t &&array)
{
    reset();
    m_storage_context = storage_context_t::create(std::move(array));
    m_storage_type = storage_type_t::stored_data;
    return *this;
}

smart_buffer &smart_buffer::assign(const i_data_object *data_object_ptr)
{
    reset();
    m_ref_const_data_pointer = data_object_ptr;
    m_storage_type = storage_type_t::ref_const_data_pointer;
    return *this;
}

smart_buffer &smart_buffer::assign(i_data_object::s_ptr_t data_object)
{
    reset();
    m_storage_context = data_object;
    m_storage_type = storage_type_t::shared_data_pointer;
    return *this;
}

bool smart_buffer::operator ==(const smart_buffer &other) const
{
    auto sz = size();
    if (sz == other.size())
    {
        const auto ptr1 = data();
        const auto ptr2 = other.data();

        return sz == 0
                || ptr1 == ptr2
                || std::memcmp(ptr1, ptr2, sz) == 0;
    }
    return false;
}

bool smart_buffer::operator !=(const smart_buffer &other) const
{
    return ! operator == (other);
}

bool smart_buffer::is_empty() const
{
    return data() == nullptr
            || size() == 0;
}

bool smart_buffer::is_valid() const
{
    return m_storage_type != storage_type_t::undefined;
}

void smart_buffer::reset()
{
    switch(m_storage_type)
    {
        case storage_type_t::ref_const_data:
            m_ref_const_data = nullptr;
            m_ref_size = 0;
        break;
        case storage_type_t::ref_const_array:
            m_ref_const_array = nullptr;
        break;
        case storage_type_t::ref_const_data_pointer:
            m_ref_const_data_pointer = nullptr;
        break;
        case storage_type_t::stored_data:
        case storage_type_t::shared_data_pointer:
            m_storage_context.reset();
        break;
    }

    m_storage_type = storage_type_t::undefined;
}

void smart_buffer::make_store()
{
    if (is_valid())
    {
        if (m_storage_type != storage_type_t::stored_data
                || m_storage_context.use_count() > 1)
        {
            assign(utils::create_raw_array(data()
                                          , size()));
        }
    }
    else
    {
        assign(raw_array_t{});
    }
}

const uint8_t &smart_buffer::operator[](int32_t index) const
{
    return *(static_cast<const std::uint8_t*>(data()) + index);
}

const void *smart_buffer::data() const
{
    switch(m_storage_type)
    {
        case storage_type_t::ref_const_data:
            return m_ref_const_data;
        break;
        case storage_type_t::ref_const_array:
            return m_ref_const_array->data();
        break;
        case storage_type_t::ref_const_data_pointer:
            return m_ref_const_data_pointer->data();
        break;
        case storage_type_t::stored_data:
        case storage_type_t::shared_data_pointer:
            return m_storage_context->data();
        break;
        default: ;
    }

    return nullptr;
}

void *smart_buffer::map()
{
    make_store();
    return static_cast<storage_context_t&>(*m_storage_context).map();
}

std::size_t smart_buffer::size() const
{
    switch(m_storage_type)
    {
        case storage_type_t::ref_const_data:
            return m_ref_size;
        break;
        case storage_type_t::ref_const_array:
            return m_ref_const_array->size();
        break;
        case storage_type_t::ref_const_data_pointer:
            return m_ref_const_data_pointer->size();
        break;
        case storage_type_t::stored_data:
        case storage_type_t::shared_data_pointer:
            return m_storage_context->size();
        break;
        default: ;
    }

    return 0;
}

i_buffer::u_ptr_t smart_buffer::clone() const
{
    if (is_valid())
    {
        if (is_shared())
        {
            return u_ptr_t(new smart_buffer(m_storage_context
                                              , m_storage_type == storage_type_t::stored_data));
        }

        return create(data()
                      , size()
                      , true);
    }

    return nullptr;
}

std::size_t smart_buffer::refs() const
{
    if (is_valid()
            && is_shared())
    {
        return m_storage_context.use_count();
    }

    return 0;
}

void smart_buffer::make_shared()
{
    if (is_valid()
            && !is_shared())
    {
        assign(data()
               , size()
               , true);
    }
}

void smart_buffer::append_data(const void *data, std::size_t size)
{
    make_store();
    return static_cast<storage_context_t&>(*m_storage_context).append(data, size);
}

void smart_buffer::resize(std::size_t new_size)
{
    make_store();
    static_cast<storage_context_t&>(*m_storage_context).resize(new_size);
}

void smart_buffer::clear()
{
    make_store();
    static_cast<storage_context_t&>(*m_storage_context).clear();
}

bool smart_buffer::is_shared() const
{
    return m_storage_type == storage_type_t::stored_data
            || m_storage_type == storage_type_t::shared_data_pointer;
}

}
