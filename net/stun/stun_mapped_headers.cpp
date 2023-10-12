#include "stun_mapped_headers.h"
#include "utils/common_utils.h"
#include "utils/convert_utils.h"
#include "utils/endian_utils.h"
// #include "utils/crypto_utils.h"
#include <cstring>


#include <iostream>

namespace mpl::net
{

#define __declare_attribute_method(type)\
    template type stun_mapped_attribute_t::get_value<type>(std::int32_t offset) const;\
    template void stun_mapped_attribute_t::set_value<type>(const type& value, std::int32_t offset);

__declare_attribute_method(std::uint8_t)
__declare_attribute_method(std::uint16_t)
__declare_attribute_method(std::uint32_t)
__declare_attribute_method(std::uint64_t)
__declare_attribute_method(std::int8_t)
__declare_attribute_method(std::int16_t)
__declare_attribute_method(std::int32_t)
__declare_attribute_method(std::int64_t)

stun_protocol_family_t stun_address_t::get_protocol_family() const
{
    switch(auto normalize_protocol_family = static_cast<stun_protocol_family_t>(protocol_family))
    {
        case stun_protocol_family_t::ip4:
        case stun_protocol_family_t::ip6:
            return normalize_protocol_family;
    }
    return stun_protocol_family_t::undefined;
}

bool stun_mapped_header_t::is_valid() const
{
    if ((type & 0xff) < 3
            && cookie == stun_message_cookie)
    {
        return true;
    }

    return false;
}

bool stun_mapped_header_t::is_optional_method() const
{
    return (type & 0x0002) != 0;
}

stun_message_class_t stun_mapped_header_t::get_class() const
{
    // TODO: need endian fixes
    std::uint16_t msg_class = ((type & 0x1000) >> 12) | ((type & 0x0001) << 1);

    return static_cast<stun_message_class_t>(msg_class);
}

void stun_mapped_header_t::set_class(stun_message_class_t stun_class)
{
    if (stun_class != stun_message_class_t::undefined)
    {
        std::uint16_t msg_class = static_cast<std::uint16_t>(stun_class);
        auto mask = ((msg_class << 12) & 0x1000) | (msg_class >> 1) & 0x0001;
        type = (type & (~0x1001)) | mask;
    }
}

stun_method_t stun_mapped_header_t::get_method() const
{
    // TODO: need endian fixes
    std::uint16_t msg_method = ((type & 0x0f00) >> 8
                                 | ((type & 0xe000) >> 7)
                                 | ((type & 0x003e) << 6));

    return msg_method == static_cast<std::uint16_t>(stun_method_t::binding)
            ? static_cast<stun_method_t>(msg_method)
            : stun_method_t::undefined;
}

void stun_mapped_header_t::set_method(stun_method_t stun_method)
{
    if (stun_method != stun_method_t::undefined)
    {
        std::uint16_t method = static_cast<std::uint16_t>(stun_method);
        auto mask = ((method << 8) & 0x0f00)
                    | ((method << 7) & 0xe000)
                    | ((method >> 6) & 0x003e);

        type = (type & 0x1001) | mask;
    }
}


stun_attribute_id_t stun_mapped_attribute_header_t::get_attribute_id() const
{
    switch(auto normalize_attribute
           = static_cast<stun_attribute_id_t>(utils::endian::big:convert(attribute)))
    {
        case stun_attribute_id_t::mapped_address:
        case stun_attribute_id_t::username:
        case stun_attribute_id_t::message_integrity:
        case stun_attribute_id_t::error_code:
        case stun_attribute_id_t::unknown:
        case stun_attribute_id_t::realm:
        case stun_attribute_id_t::nonce:
        case stun_attribute_id_t::xor_mapped_address:
        case stun_attribute_id_t::priority:
        case stun_attribute_id_t::use_candidate:
        case stun_attribute_id_t::software:
        case stun_attribute_id_t::alternate_server:
        case stun_attribute_id_t::fingerprint:
        case stun_attribute_id_t::ice_controlled:
        case stun_attribute_id_t::ice_controlling:
            return normalize_attribute;
        break;
    }

    return stun_attribute_id_t::undefined;
}

bool stun_mapped_attribute_header_t::is_optional_attribute() const
{
    return (attribute & 0x0080) != 0;
}

std::size_t stun_mapped_attribute_t::payload_size() const
{
    return utils::endian::big_endian_convert(header.length);
}

uint8_t *stun_mapped_attribute_t::payload(int32_t offset)
{
    return reinterpret_cast<std::uint8_t*>(this) + offset + sizeof(header);
}

const uint8_t *stun_mapped_attribute_t::payload(int32_t offset) const
{
    return reinterpret_cast<const std::uint8_t*>(this) + offset + sizeof(header);
}

template<typename T>
T stun_mapped_attribute_t::get_value(std::int32_t offset) const
{
    return utils::endian::get_big_endian_value<T>(payload(offset));
}

template<typename T>
void stun_mapped_attribute_t::set_value(const T& value
                                        , std::int32_t offset)
{
    return utils::endian::set_big_endian_value(value
                                               , payload(offset));
}

bool stun_mapped_message_t::is_valid(std::size_t length) const
{
    if (length >= stun_header_size)
    {
        return header.is_valid()
                && (payload_size() + stun_header_size) == length;
    }
    return false;
}

std::size_t stun_mapped_message_t::payload_size() const
{
    return utils::endian::big_endian_convert(header.length);
}

void stun_mapped_message_t::set_payload_size(std::size_t size)
{
    header.length = utils::endian::big_endian_convert<std::uint16_t>(size);
}

uint8_t *stun_mapped_message_t::payload(int32_t offset)
{
    return reinterpret_cast<std::uint8_t*>(this) + offset + sizeof(header);
}

const uint8_t *stun_mapped_message_t::payload(int32_t offset) const
{
    return reinterpret_cast<const std::uint8_t*>(this) + offset + sizeof(header);
}

stun_authentification_result_t stun_mapped_message_t::check_username(const std::string &username) const
{
    const stun_mapped_attribute_t* attribute = get_attribute(stun_attribute_id_t::username);

    if (attribute != nullptr
            && attribute->payload_size() > 0)
    {
        std::string attr_username(reinterpret_cast<const char*>(attribute->payload())
                                  , attribute->payload_size());

        if (attr_username == username)
        {
            return stun_authentification_result_t::ok;
        }

        /*
        if (attr_username.find(username) == 0
                && attr_username[username.length()] == ':')
        {
            return stun_authentification_result_t::ok;
        }*/

        return stun_authentification_result_t::unautorized;
    }

    return stun_authentification_result_t::bad_request;
}

stun_authentification_result_t stun_mapped_message_t::check_password(const std::string &password) const
{
    const stun_mapped_attribute_t* attribute = get_attribute(stun_attribute_id_t::message_integrity);

    if (attribute != nullptr
            && attribute->payload_size() == 20)
    {
        static thread_local std::vector<std::uint8_t> buffer;
        auto hash_size = reinterpret_cast<std::size_t>(attribute) - reinterpret_cast<std::size_t>(this);
        if (buffer.size() < hash_size)
        {
            buffer.resize(hash_size);
        }

        std::memcpy(buffer.data()
                    , this
                    , hash_size);

        reinterpret_cast<stun_mapped_message_t*>(buffer.data())->set_payload_size(hash_size - sizeof(stun_mapped_header_t) + 24);
        auto calc_hash = utils::crypto::hash_hmac_sha1(password
                                                       , buffer.data()
                                                       , hash_size);
        if (std::memcmp(calc_hash.data()
                        , attribute->payload()
                        , calc_hash.size()) == 0)
        {
            return stun_authentification_result_t::ok;
        }

        return stun_authentification_result_t::unautorized;
    }

    return stun_authentification_result_t::bad_request;
}

stun_mapped_attribute_t *stun_mapped_message_t::get_attribute(stun_attribute_id_t attribute_id)
{
    auto size = payload_size();
    auto ptr = payload();

    while(size >= sizeof(stun_mapped_attribute_t))
    {
        stun_mapped_attribute_t* attribute = reinterpret_cast<stun_mapped_attribute_t*>(ptr);

        if (attribute->header.get_attribute_id() == attribute_id)
        {
            return attribute;
        }

        auto asize = attribute->payload_size() + sizeof(stun_mapped_attribute_t);

        if (asize % stun_align_size != 0)
        {
            asize += stun_align_size - asize % stun_align_size;
        }

        if (asize > size)
        {
            break;
        }

        size -= asize;
        ptr += asize;
    }

    return nullptr;
}

const stun_mapped_attribute_t *stun_mapped_message_t::get_attribute(stun_attribute_id_t attribute_id) const
{
    return const_cast<stun_mapped_message_t*>(this)->get_attribute(attribute_id);
}

std::vector<stun_mapped_attribute_t *> stun_mapped_message_t::get_attributes()
{
    std::vector<stun_mapped_attribute_t *> attribute_list;

    auto size = payload_size();
    auto ptr = payload();

    while(size >= sizeof(stun_mapped_attribute_t))
    {
        stun_mapped_attribute_t* attribute = reinterpret_cast<stun_mapped_attribute_t*>(ptr);
        auto asize = attribute->payload_size() + sizeof(stun_mapped_attribute_t);

        if (asize % 4 != 0)
        {
            asize += 4 - asize % 4;
        }

        if (asize > size)
        {
            break;
        }

        attribute_list.push_back(attribute);
        size -= asize;
        ptr += asize;
    }

    if (size != 0)
    {
        attribute_list.clear();
    }

    return attribute_list;
}

std::vector<const stun_mapped_attribute_t *> stun_mapped_message_t::get_attributes() const
{
    std::vector<const stun_mapped_attribute_t *> attribute_list;

    auto size = payload_size();
    auto ptr = payload();

    while(size >= sizeof(stun_mapped_attribute_t))
    {
        const stun_mapped_attribute_t* attribute = reinterpret_cast<const stun_mapped_attribute_t*>(ptr);
        auto asize = attribute->payload_size() + sizeof(stun_mapped_attribute_t);

        if (asize % 4 != 0)
        {
            asize += 4 - asize % 4;
        }

        if (asize > size)
        {
            break;
        }

        attribute_list.push_back(attribute);
        size -= asize;
        ptr += asize;
    }

    if (size != 0)
    {
        attribute_list.clear();
    }

    return attribute_list;
}

}
