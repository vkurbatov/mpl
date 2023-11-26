#ifndef MPL_UTILS_HASH_UTILS_H
#define MPL_UTILS_HASH_UTILS_H

#include <cstdint>
#include <string_view>
#include <vector>

namespace mpl::utils
{

using hash_array_t = std::vector<std::uint8_t>;


std::size_t calc_hash(const void* data
                      , std::size_t size);

std::uint8_t calc_checksum(const void* data
                           , std::size_t size);

std::uint16_t calc_crc16(const void* data
                        , std::size_t size);

std::uint32_t calc_crc32(const void* data
                        , std::size_t size);

hash_array_t calc_sha1(const void* data
                       , std::size_t size);

hash_array_t calc_md5(const void* data
                       , std::size_t size);

hash_array_t calc_hmac_sha1(const void* data
                            , std::size_t size
                            , const std::string_view& key);


}
#endif // MPL_UTILS_HASH_UTILS_H
