#ifndef SSL_HASH_TYPES_H
#define SSL_HASH_TYPES_H

namespace ssl
{

enum class hash_method_t
{
    none = 0,
    md4,
    md5,
    sha1,
    sha224,
    sha256,
    sha384,
    sha512,
    md5_sha1
};


}

#endif // SSL_HASH_TYPES_H
