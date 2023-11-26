#ifndef MPL_NET_TLS_TYPES_H
#define MPL_NET_TLS_TYPES_H

#include <cstdint>

namespace mpl::net
{

enum class tls_hash_method_t
{
    undefined = 0,
    md4,
    md5,
    sha_1,
    sha_224,
    sha_256,
    sha_384,
    sha_512,
    md5_sha1
};

enum class tls_method_t
{
    dtls,
    tls
};

}

#endif // MPL_NET_TLS_TYPES_H
