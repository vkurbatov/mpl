#ifndef MPL_NET_STUN_MAPPED_HEADERS_H
#define MPL_NET_STUN_MAPPED_HEADERS_H

#include "stun_types.h"
#include <string>
#include <vector>

namespace mpl::net
{

#pragma pack(push, 1)

using stun_transaction_data_t = std::uint8_t[stun_transaction_data_size];

struct stun_address_t
{
    std::uint8_t    protocol_family;
    std::uint16_t   port;
    union
    {
        std::uint32_t       ip4_address;
        std::uint8_t        ip6_address[16];
    };

    stun_protocol_family_t get_protocol_family() const;
};

struct stun_mapped_attribute_header_t
{
    std::uint16_t   attribute;
    std::uint16_t   length;

    stun_attribute_id_t get_attribute_id() const;
    bool is_optional_attribute() const;
};

struct stun_mapped_attribute_t
{
    stun_mapped_attribute_header_t header;

    std::size_t payload_size() const;
    std::uint8_t* payload(std::int32_t offset = 0);
    const std::uint8_t* payload(std::int32_t offset = 0) const;

    template<typename T>
    T get_value(std::int32_t offset = 0) const;

    template<typename T>
    void set_value(const T& value, std::int32_t offset = 0);
};

struct stun_mapped_header_t
{
    std::uint16_t           type;
    std::uint16_t           length;
    std::uint32_t           cookie;
    stun_transaction_data_t transaction_id;

    bool is_valid() const;
    bool is_optional_method() const;
    stun_message_class_t get_class() const;
    void set_class(stun_message_class_t stun_class);
    stun_method_t get_method() const;
    void set_method(stun_method_t stun_method);
};

struct stun_mapped_message_t
{
    stun_mapped_header_t   header;

    bool is_valid(std::size_t length = stun_header_size) const;
    std::size_t payload_size() const;
    void set_payload_size(std::size_t size);

    std::uint8_t* payload(std::int32_t offset = 0);
    const std::uint8_t* payload(std::int32_t offset = 0) const;

    stun_authentification_result_t check_username(const std::string& username) const;

    stun_authentification_result_t check_password(const std::string& password) const;

    std::vector<std::uint8_t> create_hash() const;

    stun_mapped_attribute_t* get_attribute(stun_attribute_id_t attribute_id);
    const stun_mapped_attribute_t* get_attribute(stun_attribute_id_t attribute_id) const;

    std::vector<stun_mapped_attribute_t*> get_attributes();
    std::vector<const stun_mapped_attribute_t*> get_attributes() const;
};


#pragma pack(pop)

}

#endif // MPL_NET_STUN_MAPPED_HEADERS_H
