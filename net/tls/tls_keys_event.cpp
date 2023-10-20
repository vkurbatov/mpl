#include "tls_keys_event.h"

namespace mpl::net
{

tls_keys_event_t::tls_keys_event_t(const ssl::srtp_key_info_t &encryption_key
                                   , const ssl::srtp_key_info_t &decryption_key)
    : event_t(id
              , event_name)
    , encryption_key(encryption_key)
    , decryption_key(decryption_key)
{

}

bool tls_keys_event_t::operator ==(const tls_keys_event_t &other) const
{
    return decryption_key == other.decryption_key
            && encryption_key == other.encryption_key;
}

bool tls_keys_event_t::operator ==(const event_t &other) const
{
    return event_t::operator == (other)
            && *this == (static_cast<const tls_keys_event_t&>(other));
}

}
