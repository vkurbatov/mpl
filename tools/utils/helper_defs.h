#ifndef BASE_HELPER_DEFS_H
#define BASE_HELPER_DEFS_H

#define __enable_method_if(__own_type, __target_type) \
template<class __T = __own_type \
, class std::enable_if<std::is_same<__T, __target_type>::value, int>::type = 0>

#define __enable_method_if_2(__own_type, __target_type1, __target_type2) \
template<class __T = __own_type \
, class std::enable_if<std::is_same<__T, __target_type1>::value || std::is_same<__T, __target_type2>::value, int>::type = 0>

#define __enable_method_if_not(__own_type, __target_type) \
template<class __T = __own_type \
, class std::enable_if<!std::is_same<__T, __target_type>::value, int>::type = 0>

#define __enable_method_if_not_2(__own_type, __target_type1, __target_type2) \
template<class __T = __own_type \
, class std::enable_if<!std::is_same<__T, __target_type1>::value && !std::is_same<__T, __target_type2>::value, int>::type = 0>

#endif // BASE_HELPER_DEFS_H
