#include "stun_utils.h"
#include "stun_mapped_headers.h"

namespace mpl::utils
{

net::stun_authentification_result_t stun_verify_auth(const std::string_view &password
                                                     , const i_data_object &packet_data)
{
    auto& stun_message = *static_cast<const net::stun_mapped_message_t*>(packet_data.data());
    if (stun_message.is_valid(packet_data.size()))
    {
        return stun_message.check_password(password);
    }

    return net::stun_authentification_result_t::bad_request;
}



}
