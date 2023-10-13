#ifndef SSL_NATIVE_H
#define SSL_NATIVE_H

#include "ssl_pointers.h"

// fwd
struct ssl_ctx_st;
struct ssl_st;
struct bio_st;
struct x509_st;
struct evp_pkey_st;
struct ec_key_st;

namespace ssl
{

using ssl_ctx_ptr_t = ssl_s_ptr_t<ssl_ctx_st>;
using ssl_ptr_t = ssl_s_ptr_t<ssl_st>;
using bio_ptr_t = ssl_s_ptr_t<bio_st>;
using x509_ptr_t = ssl_s_ptr_t<x509_st>;
using evp_pkey_ptr_t = ssl_s_ptr_t<evp_pkey_st>;
using ec_key_ptr_t = ssl_unique_s_ptr_t<ec_key_st>;



}

#endif // SSL_NATIVE_H
