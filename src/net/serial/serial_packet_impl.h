#ifndef MPL_NET_SERIAL_PACKET_IMPL_H
#define MPL_NET_SERIAL_PACKET_IMPL_H

#include "i_serial_packet.h"
#include "utils/smart_buffer_container.h"
#include "utils/option_container.h"

namespace mpl::net
{

class serial_packet_impl : public i_serial_packet
        , public utils::smart_buffer_container
        , public utils::option_container
{
    std::string     m_port_name;

public:
    using u_ptr_t = std::unique_ptr<serial_packet_impl>;
    using s_ptr_t = std::shared_ptr<serial_packet_impl>;

    static u_ptr_t create(const smart_buffer& packet_buffer
                          , const std::string_view& port_name);

    static u_ptr_t create(smart_buffer&& packet_buffer = {}
                          , const std::string_view& port_name = {});

    serial_packet_impl(const smart_buffer& packet_buffer
                       , const std::string_view& port_name);

    serial_packet_impl(smart_buffer&& packet_buffer = {}
                       , const std::string_view& port_name = {});

    // i_data_object interface
public:
    const void *data() const override;
    std::size_t size() const override;

    // i_message interface
public:
    message_category_t category() const override;
    module_id_t module_id() const override;
    i_message::u_ptr_t clone() const override;

    // i_message_packet interface
public:
    const i_option *options() const override;

    // i_net_packet interface
public:
    transport_id_t transport_id() const override;
    bool is_valid() const override;

    // i_serial_packet interface
public:
    std::string port_name() const override;
};

}

#endif // MPL_NET_SERIAL_PACKET_IMPL_H
