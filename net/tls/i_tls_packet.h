#ifndef MPL_NET_I_TLS_PACKET_H
#define MPL_NET_I_TLS_PACKET_H

#include "net/i_net_packet.h"


namespace mpl::net
{

class i_tls_packet : public i_net_packet
{
public:
    enum class content_type_t
    {
        undefined = -1,
        change_cipher_spec = 20,
        alert = 21,
        handshake = 22,
        application_data = 23
    };

    using u_ptr_t = std::unique_ptr<i_tls_packet>;
    using s_ptr_t = std::shared_ptr<i_tls_packet>;

    virtual std::uint64_t sequension_number() const = 0;
    virtual content_type_t content_type() const = 0;
};

}

#endif // MPL_NET_I_TLS_PACKET_H
