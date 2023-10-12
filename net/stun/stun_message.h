#ifndef MPL_NET_STUN_MESSAGE_H
#define MPL_NET_STUN_MESSAGE_H

#include "stun_attributes.h"
#include "utils/smart_buffer.h"
#include <array>

namespace mpl::net
{

struct stun_message_t
{

    static bool parse(const i_data_object& parse_data
                      , stun_message_t& message);

    static stun_transaction_id_t generate_transaction_id();

    stun_message_class_t                message_class;
    stun_method_t                       method;
    stun_transaction_id_t               transaction_id;
    stun_attribute_t::w_ptr_list_t      attributes;

    stun_message_t(stun_message_class_t message_class = stun_message_class_t::undefined
                      , stun_method_t method = stun_method_t::undefined
                      , const stun_transaction_id_t& transaction_id = generate_transaction_id()
                      , const stun_attribute_t::w_ptr_list_t& attributes = {});

    stun_attribute_t::s_ptr_t get_attribute(stun_attribute_id_t attribute_id);

    bool is_valid() const;
    void reset();
    smart_buffer build_packet(const std::string& password = {}) const;
    std::string to_string() const;

};

}

#endif // MPL_NET_STUN_MESSAGE_H
