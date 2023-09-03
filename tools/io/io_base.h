#ifndef IO_BASE_H
#define IO_BASE_H

#include <functional>
#include <cstdint>
#include <vector>
#include <string>

namespace io
{

using raw_array_t = std::vector<std::uint8_t>;

enum class link_type_t
{
    undefined = -1,
    serial,
    ip,
    udp,
    tcp,
    tcp_listener,
    ux,
    pipe
};

enum class endpoint_type_t
{
    undefined = -1,
    serial,
    ip,
    udp,
    tcp,
    ux,
    pipe
};

enum class link_state_t
{
    undefined = -1,
    ready,
    opening,
    open,
    connecting,
    connected,
    disconnecting,
    disconnected,
    closing,
    closed,
    failed
};

enum class link_control_id_t
{
    undefined,
    open,
    close,
    start,
    stop
};

struct link_config_t
{
    link_type_t type;

    link_config_t();

    virtual bool is_valid() const;

protected:
    link_config_t(link_type_t type);
};

struct endpoint_t
{
    static const endpoint_t& undefined();
    endpoint_type_t type;
    endpoint_t();

    virtual bool operator == (const endpoint_t& other) const;
    virtual bool operator != (const endpoint_t& other) const;
    virtual bool is_valid() const;

protected:
    endpoint_t(endpoint_type_t type);
};


class message_t
{
    const void*     m_ref_data;
    std::size_t     m_ref_size;
    raw_array_t     m_store_data;

public:
    message_t(const void* data = nullptr
              , std::size_t size = 0
              , bool store = false);

    message_t(raw_array_t&& raw_data);

    void assign(const void* data = nullptr
            , std::size_t size = 0
            , bool store = false);

    void assign(raw_array_t&& raw_data);

    raw_array_t release();
    void make_store();
    bool is_stored() const;
    const void* data() const;
    void* map();
    std::size_t size() const;
    void clear();
    bool is_empty() const;

};

using executor_handler_t  = std::function<void()>;
using message_handler_t = std::function<void(const message_t& message, const endpoint_t& endpoint)>;
using state_handler_t = std::function<void(link_state_t state, const std::string_view& reason)>;

}

#endif // IPC_BASE_H
