#ifndef MPL_NET_TLS_FINGERPRINT_H
#define MPL_NET_TLS_FINGERPRINT_H

#include "tls_types.h"
#include <vector>
#include <string>

namespace mpl::net
{

struct tls_fingerprint_t
{
    using hash_t = std::vector<std::uint8_t>;

    tls_hash_method_t   method;
    hash_t              hash;

    tls_fingerprint_t(tls_hash_method_t method = tls_hash_method_t::undefined
                      , const hash_t& hash = {});

    bool operator == (const tls_fingerprint_t& other) const;
    bool operator != (const tls_fingerprint_t& other) const;

    bool is_compatible(const tls_fingerprint_t& other) const;

    bool is_defined() const;

    bool from_string(const std::string& params_line);
    std::string to_string() const;
};

}

#endif // MPL_NET_TLS_FINGERPRINT_H
