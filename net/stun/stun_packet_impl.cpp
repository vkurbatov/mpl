#include "stun_packet_impl.h"
#include "stun_mapped_headers.h"
#include "stun_attributes.h"
#include "stun_message.h"

#include <cstring>

namespace mpl::net
{

stun_packet_impl::u_ptr_t stun_packet_impl::create(const smart_buffer &buffer
                                                   , const socket_address_t& address)
{
    return std::make_unique<stun_packet_impl>(buffer
                                              , address);
}

stun_packet_impl::u_ptr_t stun_packet_impl::create(smart_buffer &&buffer
                                                   , const socket_address_t& address)
{
    return std::make_unique<stun_packet_impl>(std::move(buffer)
                                              , address);
}

stun_packet_impl::stun_packet_impl(const smart_buffer &buffer
                                   , const socket_address_t& address)
    : smart_buffer_container(buffer)
    , m_address(address)
{

}

stun_packet_impl::stun_packet_impl(smart_buffer &&buffer
                                   , const socket_address_t& address)
    : smart_buffer_container(std::move(buffer))
    , m_address(address)
{

}

void stun_packet_impl::set_address(const socket_address_t &address)
{
    m_address = address;
}

message_category_t stun_packet_impl::category() const
{
    return message_category_t::packet;
}

message_subclass_t stun_packet_impl::subclass() const
{
    return message_net_class;
}

bool stun_packet_impl::is_valid() const
{
    return mapped_message().is_valid(size());
}

const void *stun_packet_impl::data() const
{
    return m_buffer.data();
}

std::size_t stun_packet_impl::size() const
{
    return m_buffer.size();
}

transport_id_t stun_packet_impl::transport_id() const
{
    return transport_id_t::ice;
}

const i_option* stun_packet_impl::options() const
{
    return &m_options;
}

const void *stun_packet_impl::payload_data() const
{
    return mapped_message().payload();
}

std::size_t stun_packet_impl::payload_size() const
{
    return mapped_message().payload_size();
}

i_message::u_ptr_t stun_packet_impl::clone() const
{
   if (auto clone_packet = create(m_buffer.fork()))
   {
       clone_packet->set_options(m_options);
       return clone_packet;
   }

   return nullptr;
}

stun_message_class_t stun_packet_impl::stun_class() const
{
    return mapped_message().header.get_class();
}

stun_method_t stun_packet_impl::stun_method() const
{
    return mapped_message().header.get_method();
}

stun_transaction_id_t stun_packet_impl::transaction_id() const
{
    stun_transaction_id_t transaction_id = {};
    if (mapped_message().is_valid(size()))
    {
        std::memcpy(transaction_id.data()
                    , mapped_message().header.transaction_id
                    , stun_transaction_data_size);
    }
    return transaction_id;
}

stun_attribute_t::s_ptr_list_t stun_packet_impl::attributes() const
{
    if (is_valid())
    {
        stun_message_t message;
        if (stun_message_t::parse(m_buffer
                                  , message))
        {
            return std::move(message.attributes);
        }
    }
    return {};
}

const socket_address_t &stun_packet_impl::address() const
{
    return m_address;
}

const stun_mapped_message_t &stun_packet_impl::mapped_message() const
{
    return *static_cast<const stun_mapped_message_t*>(data());
}

const uint8_t *stun_packet_impl::map(int32_t index) const
{
    return static_cast<const std::uint8_t*>(data()) + index;
}

}
