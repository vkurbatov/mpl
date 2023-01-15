#include "option_value_impl.h"
#include "common_types.h"

#include <string>

namespace mpl
{

// forward basic type declarations
template class option_value_impl<std::int8_t>;
template class option_value_impl<std::int16_t>;
template class option_value_impl<std::int32_t>;
template class option_value_impl<std::int64_t>;
template class option_value_impl<std::uint8_t>;
template class option_value_impl<std::uint16_t>;
template class option_value_impl<std::uint32_t>;
template class option_value_impl<std::uint64_t>;
template class option_value_impl<float>;
template class option_value_impl<double>;
template class option_value_impl<long double>;
template class option_value_impl<bool>;
template class option_value_impl<std::string>;
template class option_value_impl<octet_string_t>;

}
