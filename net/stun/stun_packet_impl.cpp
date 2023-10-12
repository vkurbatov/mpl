#include "stun_packet_impl.h"
#include "stun_mapped_headers.h"
#include "stun_attributes.h"
#include "stun_message.h"

#include <cstring>

namespace mpl::net
{

stun_packet_impl::u_ptr_t stun_packet_impl::create(const smart_buffer &buffer)
{
    return std::make_unique<stun_packet_impl>(buffer);
}

stun_packet_impl::u_ptr_t stun_packet_impl::create(smart_buffer &&buffer)
{
    return std::make_unique<stun_packet_impl>(std::move(buffer));
}

stun_packet_impl::stun_packet_impl(const smart_buffer &buffer)
    : rtc_buffer_container(buffer)
{

}

stun_packet_impl::stun_packet_impl(smart_buffer &&buffer)
    : rtc_buffer_container(std::move(buffer))
{

}

rtc_message_category_t stun_packet_impl::category() const
{
    return rtc_message_category_t::packet;
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

rtc_packet_type_t stun_packet_impl::packet_type() const
{
    return rtc_packet_type_t::stun;
}

const i_option &stun_packet_impl::options() const
{
    return m_options;
}

const void *stun_packet_impl::payload_data() const
{
    return mapped_message().payload();
}

std::size_t stun_packet_impl::payload_size() const
{
    return mapped_message().payload_size();
}

i_rtc_message::u_ptr_t stun_packet_impl::clone() const
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

stun_attribute_t::w_ptr_list_t stun_packet_impl::attributes() const
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

stun_authentification_result_t stun_packet_impl::check_authentification(const std::string &password) const
{
    if (!password.empty())
    {
        return mapped_message().check_password(password);
    }
    return stun_authentification_result_t::ok;
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
