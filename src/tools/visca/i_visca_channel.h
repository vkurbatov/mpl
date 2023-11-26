#ifndef I_VISCA_CHANNEL_H
#define I_VISCA_CHANNEL_H

#include <vector>
#include <cstdint>

namespace pt::visca
{

class i_visca_channel
{
public:
    using packet_data_t = std::vector<std::uint8_t>;

    virtual ~i_visca_channel() = default;
    virtual bool open() = 0;
    virtual bool close() = 0;
    virtual std::size_t write(const void* data, std::size_t size) = 0;
    virtual std::size_t read(packet_data_t& data
                             , std::uint32_t timeout = 0) = 0;
    virtual bool is_open() const = 0;
    virtual bool flush() = 0;
};

}

#endif // I_VISCA_CHANNEL_H
