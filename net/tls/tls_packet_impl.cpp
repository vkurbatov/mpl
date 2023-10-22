#include "tls_packet_impl.h"

#include "net/net_message_types.h"

#include "tools/ssl/mapped_dtls_header.h"


namespace mpl::net
{

tls_packet_impl::u_ptr_t tls_packet_impl::create(const smart_buffer &buffer)
{
    return std::make_unique<tls_packet_impl>(std::move(buffer));
}

tls_packet_impl::u_ptr_t tls_packet_impl::create(smart_buffer &&buffer)
{
    return std::make_unique<tls_packet_impl>(std::move(buffer));
}

tls_packet_impl::tls_packet_impl(const smart_buffer &buffer)
    : smart_buffer_container(buffer)
{

}

tls_packet_impl::tls_packet_impl(smart_buffer &&buffer)
    : smart_buffer_container(std::move(buffer))
{

}

const void *tls_packet_impl::data() const
{
    return m_buffer.data();
}

std::size_t tls_packet_impl::size() const
{
    return m_buffer.size();
}

message_category_t tls_packet_impl::category() const
{
    return message_category_t::packet;
}

message_subclass_t tls_packet_impl::subclass() const
{
    return message_class_net;
}

i_message::u_ptr_t tls_packet_impl::clone() const
{
    if (auto clone_packet = create(m_buffer.fork()))
    {
        clone_packet->set_options(m_options);
        return clone_packet;
    }

    return nullptr;
}

const i_option *tls_packet_impl::options() const
{
    return &m_options;
}

transport_id_t tls_packet_impl::transport_id() const
{
    return transport_id_t::tls;
}

bool tls_packet_impl::is_valid() const
{
    return m_buffer.size() >= sizeof(pt::ssl::mapped_dtls_header_t)
            && static_cast<const pt::ssl::mapped_dtls_header_t*>(m_buffer.data())->is_valid();
}

uint64_t tls_packet_impl::sequension_number() const
{
    return static_cast<const pt::ssl::mapped_dtls_header_t*>(m_buffer.data())->get_seq_number();
}

i_tls_packet::content_type_t tls_packet_impl::content_type() const
{
    return static_cast<i_tls_packet::content_type_t>(static_cast<const pt::ssl::mapped_dtls_header_t*>(m_buffer.data())->get_content_type());
}

}
