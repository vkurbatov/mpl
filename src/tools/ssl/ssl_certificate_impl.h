#ifndef SSL_CERTIFICATE_IMPL_H
#define SSL_CERTIFICATE_IMPL_H

#include "i_ssl_certificate.h"
#include "ssl_x509.h"

namespace pt::ssl
{

class ssl_certificate_impl : public i_ssl_certificate
{
    ssl_x509    m_x509;
public:

    using u_ptr_t = std::unique_ptr<ssl_certificate_impl>;
    using s_ptr_t = std::shared_ptr<ssl_certificate_impl>;

    static u_ptr_t create(x509_ptr_t&& x509 = {});

    ssl_certificate_impl(x509_ptr_t&& x509 = {});

    ssl_x509& x509();
    const ssl_x509& x509() const;

    // i_ssl_certificate interface
public:
    fingerprint_t get_fingerprint(hash_method_t hash_method) const override;
    bool is_valid() const override;
};

}

#endif // SSL_CERTIFICATE_IMPL_H
