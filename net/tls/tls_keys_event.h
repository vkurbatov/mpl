#ifndef MPL_NET_TLS_KEYS_EVENT_H
#define MPL_NET_TLS_KEYS_EVENT_H

#include "core/event.h"
#include "net/net_event_types.h"
#include "tools/ssl/srtp_key_info.h"
#include "tls_types.h"

namespace mpl::net
{

struct tls_keys_event_t : public event_t
{
    constexpr static event_id_t id = net_tls_keys_event_id;
    constexpr static std::string_view event_name = "tls_keys_event";

    ssl::srtp_key_info_t     encryption_key;
    ssl::srtp_key_info_t     decryption_key;

    tls_keys_event_t(const ssl::srtp_key_info_t& encryption_key
                     , const ssl::srtp_key_info_t& decryption_key);

    bool operator == (const tls_keys_event_t& other) const;


    // event_t interface
public:
    bool operator ==(const event_t &other) const override;
};

}
#endif // MPL_NET_TLS_KEYS_EVENT_H
