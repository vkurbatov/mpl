#ifndef PORTABLE_BITSTREAM_BASE_H
#define PORTABLE_BITSTREAM_BASE_H

#include <cstdint>

namespace pt::utils
{

namespace utils
{

bool get_bit(const void* src_stream
             , std::int32_t src_bit_index);

void set_bit(void* dst_stream
             , std::int32_t bit_index
             , bool value);

void copy_bits(const void* src_stream
               , void* dst_stream
               , std::int32_t src_idx
               , std::int32_t dst_idx
               , std::size_t bits);

class bit_stream_reader
{
    const void*			m_bit_stream;
    std::size_t         m_bit_count;
    std::int32_t        m_bit_index;
    bool                m_big_endian_bits;


public:
    static void read_bits(const void* bit_stream
                          , std::int32_t bit_index
                          , void* bit_data
                          , std::size_t bit_count
                          , bool big_endian = false);

public:
    bit_stream_reader(const void* bit_stream
                      , std::size_t bit_count
                      , bool big_endian_bits = false);

    bool pop(void* bit_data, std::size_t bit_count);
    bool read(void* bit_data, std::size_t bit_count) const;
    std::size_t pop_golomb(std::uint32_t& value);
    std::size_t pop_golomb(std::int32_t& value);

    std::int32_t pos() const;
    std::size_t panding() const;
    bool reset(std::int32_t bit_index = 0);
    bool skip(std::int32_t bit_shift);
};

//------------------------------------------------------------------

class bit_stream_writer
{
    void*                   m_bit_stream;
    std::size_t             m_bit_count;
    std::int32_t            m_bit_index;
    bool                    m_big_endian_bits;


public:
    static void write_bits(void* bit_stream
                           , std::int32_t bit_index
                           , const void* bit_data
                           , std::size_t bit_count
                           , bool big_endian = false);
public:
    bit_stream_writer(void* bit_stream
                      , std::size_t bit_count
                      , bool big_endian_bits = false);

    bool push(const void* bit_data, std::size_t bit_count);
    bool write(const void* bit_data, std::size_t bit_count);

    std::size_t push_golomb(std::uint32_t value);

    std::int32_t pos() const;
    std::size_t pending() const;
    bool reset(std::int32_t bit_index = 0);
    bool skip(std::int32_t bit_shift);
};

//------------------------------------------------------------------

class bit_converter
{
    void*			m_bit_stream;
public:

    static void reverse_bits(void* bit_stream
                             , int32_t bit_index
                             , std::size_t bit_count);

    static void reverse_endian(void* bit_stream
                               , int32_t byte_index
                               , std::size_t byte_count);

    bit_converter(void* bit_stream);

    void reverse_bits(int32_t bit_index, std::size_t bit_count);
    void reverse_endian(int32_t byte_index, std::size_t byte_count);
};


}

}

#endif // PORTABLE_BITSTREAM_BASE_H
