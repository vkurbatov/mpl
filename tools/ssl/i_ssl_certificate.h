#ifndef I_SSL_CERTIFICATE_H
#define I_SSL_CERTIFICATE_H

#include "ssl_types.h"
#include <memory>
#include <vector>

namespace ssl
{

class i_ssl_certificate
{
public:
    using fingerprint_t = std::vector<std::uint8_t>;
    using u_ptr_t = std::unique_ptr<i_ssl_certificate>;
    using s_ptr_t = std::shared_ptr<i_ssl_certificate>;

    virtual ~i_ssl_certificate() = default;
    virtual fingerprint_t get_fingerprint(hash_method_t hash_method) const = 0;
    virtual bool is_valid() const = 0;

};

}

#endif // I_SSL_CERTIFICATE_H
