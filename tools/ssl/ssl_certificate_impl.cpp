#include "ssl_certificate_impl.h"


namespace ssl
{

ssl_certificate_impl::u_ptr_t ssl_certificate_impl::create(x509_ptr_t &&x509)
{
    return std::make_unique<ssl_certificate_impl>(std::move(x509));
}

ssl_certificate_impl::ssl_certificate_impl(x509_ptr_t &&x509)
    : m_x509(std::move(x509))
{

}

ssl_x509 &ssl_certificate_impl::x509()
{
    return m_x509;
}

const ssl_x509 &ssl_certificate_impl::x509() const
{
    return m_x509;
}

i_ssl_certificate::fingerprint_t ssl_certificate_impl::get_fingerprint(hash_method_t hash_method) const
{
    fingerprint_t hash;
    if (m_x509.is_valid()
            && hash_method != hash_method_t::none)
    {
        m_x509.digest(hash_method
                      , hash);
    }

    return hash;
}

bool ssl_certificate_impl::is_valid() const
{
    return m_x509.is_valid();
}

}
