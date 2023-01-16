#ifndef MPL_TYPE_INFO_H
#define MPL_TYPE_INFO_H

#include <typeinfo>

namespace mpl
{

struct type_info_t
{
    using type_id_t = std::size_t;
    type_id_t             type_id;
    const std::type_info& info;

    template<typename T>
    static const type_info_t& get_type_info()
    {
        static type_info_t type_info(generate_type_id()
                                     , typeid(T));
        return type_info;
    }

private:

    static type_id_t generate_type_id();

    type_info_t(type_id_t type_id
                , const std::type_info& info);

};

}

#endif // MPL_TYPE_INFO_H
