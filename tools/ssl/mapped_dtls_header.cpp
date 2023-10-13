#include "mapped_dtls_header.h"
#include "ssl_utils.h"

#include <bit>
#include <algorithm>

namespace ssl
{

namespace
{
    void normalize(void* data
                   , std::size_t size)
    {
#if LITTLE_ENDIAN
        std::reverse(static_cast<std::uint8_t*>(data)
                     , static_cast<std::uint8_t*>(data) + size);
#endif
    }

    void normalize(const void* data_src
                   , std::size_t size
                   , void* data_dst)
    {
#if LITTLE_ENDIAN
        std::reverse_copy
#else
        std::copy
#endif
                (static_cast<const std::uint8_t*>(data_src)
                          , static_cast<const std::uint8_t*>(data_src) + size
                          , static_cast<std::uint8_t*>(data_dst));
    }

    template<typename T>
    T normalize(const T& value)
    {
        T result = value;
        normalize(&result, sizeof(T));
        return result;
    }

}

ssl_content_type_t mapped_dtls_header_t::get_content_type() const
{
    return convert<ssl_content_type_t>(normalize(content_type));
}

ssl_version_t mapped_dtls_header_t::get_version() const
{
    return convert<ssl_version_t>(normalize(version));
}

uint64_t mapped_dtls_header_t::get_seq_number() const
{
    std::uint64_t sq_number = 0;
    normalize(&sequence_number[0]
              , sizeof(sequence_number)
              , &sq_number);

    return sq_number;
}

uint16_t mapped_dtls_header_t::get_lenght() const
{
    return normalize(length);
}

bool mapped_dtls_header_t::is_valid() const
{
    return content_type > 19
            && content_type < 24
            && (version == 0xfdfe
                || version == 0xfffe);
}

const mapped_handshake_header_t *mapped_dtls_header_t::get_handshake_header() const
{
    if (get_lenght() <= sizeof(mapped_handshake_header_t))
    {
        return reinterpret_cast<const mapped_handshake_header_t*>(reinterpret_cast<const std::uint8_t*>(this) + sizeof(mapped_dtls_header_t));
    }
    return nullptr;
}

}
