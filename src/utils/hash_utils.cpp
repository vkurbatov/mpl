#include "hash_utils.h"

#include <boost/functional/hash.hpp>
#include <boost/crc.hpp>
#include <boost/uuid/detail/md5.hpp>
#include <boost/uuid/detail/sha1.hpp>

#include <openssl/sha.h>
#include <openssl/hmac.h>

namespace mpl::utils
{

namespace detail
{

template<typename Method, typename T>
inline T calc_checksum(const void *data
                        , std::size_t size)
{
    Method processor;
    processor.process_bytes(data
                            , size);
    return processor.checksum();
}

template<typename Method, typename T>
inline T calc_hash(const void *data
                   , std::size_t size)
{
    T result(sizeof(typename Method::digest_type));

    Method processor;
    processor.process_bytes(data
                            , size);

    typename Method::digest_type digets;

    processor.get_digest(digets);
    std::memcpy(result.data()
                , digets
                , result.size());

    return result;
}

HMAC_CTX* get_hmac_context()
{
    static thread_local auto hmac_context = HMAC_CTX_new();
    return hmac_context;
}

hash_array_t hash_hmac(const void *data
                       , std::size_t size
                       , const std::string_view &key
                       , const EVP_MD* method)
{
    if (auto hmac = get_hmac_context())
    {
        if (HMAC_Init_ex(hmac
                         , key.data()
                         , key.length()
                         , method
                         , nullptr) == 1)
        {

            std::uint32_t result_size = 0;

            hash_array_t hash(SHA_DIGEST_LENGTH);

            HMAC_Update(hmac
                        , static_cast<const std::uint8_t*>(data)
                        , size);
            if (HMAC_Final(hmac
                           , hash.data()
                           , &result_size) == 1
                    && result_size == hash.size())
            {
                return hash;
            }
        }
    }

    return {};
}

}

std::size_t calc_hash(const void *data
                      , std::size_t size)
{
    return boost::hash_range(static_cast<const std::uint8_t*>(data)
                             , static_cast<const std::uint8_t*>(data) + size);
}

uint8_t calc_checksum(const void *data
                      , std::size_t size)
{
    uint8_t cs = 0;
    auto ptr = static_cast<const std::uint8_t*>(data);
    while (size--)
    {
        cs += *ptr;
        ptr++;
    }

    return cs;
}

uint16_t calc_crc16(const void *data
                    , std::size_t size)
{
    return detail::calc_checksum<boost::crc_16_type
                                 , std::uint16_t>(data
                                                 , size);
}

uint32_t calc_crc32(const void *data
                    , std::size_t size)
{
    return detail::calc_checksum<boost::crc_32_type
                                 , std::uint32_t>(data
                                                 , size);
}

hash_array_t calc_sha1(const void *data
                       , std::size_t size)
{
    return detail::calc_hash<boost::uuids::detail::sha1
                                 , hash_array_t>(data
                                                 , size);
}

hash_array_t calc_md5(const void *data
                      , std::size_t size)
{
    return detail::calc_hash<boost::uuids::detail::md5
                                 , hash_array_t>(data
                                                 , size);
}

hash_array_t calc_hmac_sha1(const void *data
                            , std::size_t size
                            , const std::string_view& key)
{
    return detail::hash_hmac(data
                             , size
                             , key
                             , EVP_sha1());
}


}
