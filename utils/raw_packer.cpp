#include "raw_packer.h"
#include <cstring>

namespace mpl::utils
{

raw_packer::raw_packer(raw_array_t &array)
    : m_array(array)
{

}

raw_packer &raw_packer::append(const void *data
                                           , std::size_t size
                                           , bool reverse)
{
    if (data != nullptr)
    {
        if (!reverse)
        {
            m_array.insert(m_array.end()
                             , static_cast<const raw_array_t::value_type*>(data)
                             , static_cast<const raw_array_t::value_type*>(data) + size);
        }
        else
        {
            while(size-- > 0)
            {
                m_array.push_back(*(static_cast<const std::uint8_t*>(data) + size));
            }
        }
    }
    else
    {
        m_array.resize(m_array.size() + size);
    }

    return *this;
}

raw_packer &raw_packer::padding(std::size_t count
                                            , raw_array_t::value_type padding_value)
{
    m_array.insert(m_array.end()
                    , count
                    , padding_value);
    return *this;
}

raw_packer &raw_packer::write(int32_t idx
                                          , const void *data
                                          , std::size_t size)
{
    if ((idx + size) > m_array.size())
    {
        m_array.resize(idx + size);
    }

    std::memcpy(&m_array[idx]
                , data
                , size);

    return *this;
}

raw_packer& raw_packer::reset()
{
    m_array.clear();
    return *this;
}

}
