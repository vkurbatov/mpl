#ifndef RAW_PACKER_H
#define RAW_PACKER_H

#include "core/common_types.h"

namespace mpl::utils
{

class raw_packer
{
    raw_array_t&    m_array;
public:
    raw_packer(raw_array_t& array);
    raw_packer& append(const void* data
                              , std::size_t size
                              , bool reverse = false);

    template<typename T>
    raw_packer& append(const T& value)
    {
        return append(&value
                      , sizeof(value));
    }

    raw_packer& padding(std::size_t count
                               , raw_array_t::value_type padding_value = {});

    raw_packer& write(std::int32_t idx
                            , const void* data
                            , std::size_t size);

    raw_packer& reset();
};

}

#endif // RAW_PACKER_H
