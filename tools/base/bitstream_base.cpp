#include "bitstream_base.h"
#include "endian_base.h"
#include <algorithm>
#include <bit>
#include <cstring>

namespace portable
{

namespace utils
{

constexpr std::int32_t bit_per_byte = 8;

namespace detail
{

inline std::uint8_t get_byte_index(std::uint8_t index)
{
    return index >> 3;
}

inline std::uint8_t get_bit_index(std::uint8_t index, bool be)
{
    return be ? (7 - (index & 0x07)) : (index & 0x07);
}

inline bool get_bit(const std::uint8_t* bit_src_data
                    , uint32_t bit_src_idx
                    , bool be = false)
{
    return (bit_src_data[get_byte_index(bit_src_idx)] & (1 << get_bit_index(bit_src_idx, be))) != 0;
}

inline void set_bit(std::uint8_t* bit_dst_data
                    , uint32_t bit_dst_idx
                    , bool value
                    , bool be = false)
{
    if (value == false)
    {
        bit_dst_data[get_byte_index(bit_dst_idx)] &= ~(1 << get_bit_index(bit_dst_idx, be));
    }
    else
    {
        bit_dst_data[get_byte_index(bit_dst_idx)] |= (1 << get_bit_index(bit_dst_idx, be));
    }
}

inline void copy_bit(const std::uint8_t* bit_src_data
                     , uint32_t bit_src_idx
                     , std::uint8_t* bit_dst_data
                     , uint32_t bit_dst_idx = 0
                     , bool src_be = false
                     , bool dst_be = false)
{
    set_bit(bit_dst_data, bit_dst_idx, get_bit(bit_src_data, bit_src_idx, src_be), dst_be);
}

inline std::size_t count_bits(std::uint64_t value)
{
    std::size_t bit_count = 0;

    while (value != 0)
    {
        bit_count++;
        value >>= 1;
    }

    return bit_count;
}

void copy_bits(const void* bit_src_data
               , int32_t bit_src_idx
               , void* bit_dst_data
               , int32_t bit_dst_idx = 0
               , std::size_t count = 1
               , bool src_be = false
               , bool dst_be = false)
{
    auto src_data = static_cast<const std::uint8_t*>(bit_src_data);
    auto dst_data = static_cast<std::uint8_t*>(bit_dst_data);

    if (src_be)
    {
        bit_src_idx += count - 1;
    }
    if (dst_be)
    {
        bit_dst_idx += count - 1;
    }

    while (count-- > 0)
    {
        auto value = get_bit(src_data, bit_src_idx, src_be);
        set_bit(dst_data, bit_dst_idx, value, dst_be);

        src_be ? bit_src_idx-- : bit_src_idx++;
        dst_be ? bit_dst_idx-- : bit_dst_idx++;
    }
}

}

void bit_stream_reader::read_bits(const void *bit_stream
                                  , int32_t bit_index
                                  , void *bit_data
                                  , std::size_t bit_count
                                  , bool big_endian)
{
    detail::copy_bits(bit_stream
                                , bit_index
                                , bit_data
                                , 0
                                , bit_count
                                , big_endian
                                , false);
}

bit_stream_reader::bit_stream_reader(const void *bit_stream
                                     , std::size_t bit_count
                                     , bool big_endian_bits)
    : m_bit_stream(bit_stream)
    , m_bit_count(bit_count)
    , m_bit_index(0)
    , m_big_endian_bits(big_endian_bits)
{

}

bool bit_stream_reader::pop(void *bit_data
                            , std::size_t bit_count)
{
    if (read(bit_data
             , bit_count))
    {
        skip(bit_count);
        return true;
    }

    return false;
}

bool bit_stream_reader::read(void *bit_data
                             , std::size_t bit_count) const
{
    if ((m_bit_index + bit_count) <= m_bit_count)
    {
        read_bits(m_bit_stream
                  , m_bit_index
                  , bit_data
                  , bit_count
                  , m_big_endian_bits);
        return true;
    }

    return false;
}

std::size_t bit_stream_reader::pop_golomb(std::uint32_t& value)
{
    std::size_t bit_count = 0;
    auto save_pos = pos();

    std::uint32_t bit = 0;

    while(read(&bit, 1)
          && (bit == 0))
    {
        bit_count++;
        skip(1);
    }

    bit_count++;

    if (bit_count <= 32
            && pop(&value, bit_count))
    {
        value --;
        bit_count = bit_count * 2 - 1;
    }
    else
    {
        reset(save_pos);
        bit_count = 0;
    }
    return bit_count;
}

std::size_t bit_stream_reader::pop_golomb(int32_t &value)
{
    std::uint32_t u_value;
    auto bit_count = pop_golomb(u_value);
    if (bit_count > 0)
    {
        if ((u_value & 1) == 0)
        {
            value = -static_cast<std::int32_t>(u_value / 2);
        }
        else
        {
            value = static_cast<std::int32_t>((u_value + 1) / 2);
        }
    }

    return bit_count;
}

int32_t bit_stream_reader::pos() const
{
    return m_bit_index;
}

std::size_t bit_stream_reader::panding() const
{
    return m_bit_count - m_bit_index;
}

bool bit_stream_reader::reset(int32_t bit_index)
{
    if (bit_index >= 0 &&
            bit_index <= m_bit_count)
    {
        m_bit_index = bit_index;
        return true;
    }

    return false;
}

bool bit_stream_reader::skip(int32_t bit_shift)
{
    return reset(m_bit_index + bit_shift);
}

void bit_stream_writer::write_bits(void *bit_stream
                                   , int32_t bit_index
                                   , const void *bit_data
                                   , std::size_t bit_count
                                   , bool big_endian)
{
    detail::copy_bits(bit_data
                      , 0
                      , bit_stream
                      , bit_index
                      , bit_count
                      , false
                      , big_endian);
}

bit_stream_writer::bit_stream_writer(void *bit_stream
                                     , std::size_t bit_count
                                     , bool big_endian_bits)
    : m_bit_stream(bit_stream)
    , m_bit_count(bit_count)
    , m_bit_index(0)
    , m_big_endian_bits(big_endian_bits)
{

}

bool bit_stream_writer::push(const void *bit_data
                             , std::size_t bit_count)
{
    if (write(bit_data
            , bit_count))
    {
        skip(bit_count);
        return true;
    }

    return false;
}

bool bit_stream_writer::write(const void *bit_data
                             , std::size_t bit_count)
{
    if ((m_bit_index + bit_count) <= m_bit_count)
    {
        write_bits(m_bit_stream
                   , m_bit_index
                   , bit_data
                   , bit_count
                   , m_big_endian_bits);
        return true;
    }

    return false;
}

std::size_t bit_stream_writer::push_golomb(uint32_t value)
{
    std::uint64_t golomb_value = static_cast<std::uint64_t>(value) + 1;
    std::size_t bit_count = detail::count_bits(golomb_value) * 2 - 1;
    if (push(&golomb_value, bit_count))
    {
        return bit_count;
    }
    return 0;
}

int32_t bit_stream_writer::pos() const
{
    return m_bit_index;
}

std::size_t bit_stream_writer::pending() const
{
    return m_bit_count - m_bit_index;
}

bool bit_stream_writer::reset(int32_t bit_index)
{
    if (bit_index > 0
            && bit_index <= m_bit_count)
    {
        m_bit_index = bit_index;
        return true;
    }

    return false;
}

bool bit_stream_writer::skip(int32_t bit_shift)
{
    return reset(m_bit_index + bit_shift);
}

void bit_converter::reverse_bits(void *bit_stream
                                 , int32_t bit_index
                                 , std::size_t bit_count)
{
    auto bit_data = static_cast<std::uint8_t*>(bit_stream);
    auto end_index = bit_index + bit_count - 1;

    while (bit_index < end_index)
    {
            auto tmp_bit = detail::get_bit(bit_data, bit_index);

            detail::set_bit(bit_data, bit_index, detail::get_bit(bit_data, end_index));
            detail::set_bit(bit_data, end_index, tmp_bit);

            bit_index++;
            end_index--;
    }
}

void bit_converter::reverse_endian(void *bit_stream
                                   , int32_t byte_index
                                   , std::size_t byte_count)
{
    auto bit_data = static_cast<std::uint8_t*>(bit_stream);

    while(byte_count-- > 0)
    {
        reverse_bits(bit_data, byte_index * bit_per_byte, bit_per_byte);
        byte_index++;
    }
}

bit_converter::bit_converter(void *bit_stream)
    : m_bit_stream(bit_stream)
{

}

void bit_converter::reverse_bits(int32_t bit_index
                                 , std::size_t bit_count)
{
    reverse_bits(m_bit_stream, bit_index, bit_count);
}

void bit_converter::reverse_endian(int32_t byte_index
                                   , std::size_t byte_count)
{
    reverse_endian(m_bit_stream, byte_index, byte_count);
}

}

}
