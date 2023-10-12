#include "stun_attributes.h"
#include <unordered_map>

#include "utils/common_utils.h"

namespace mpl::net
{

stun_attribute_t::stun_attribute_t(stun_attribute_id_t attribute_id)
    : attribute_id(attribute_id)
{

}

std::string stun_attribute_t::to_string() const
{
    static const std::unordered_map<stun_attribute_id_t
            , std::string> conversion_table =
    {
        { stun_attribute_id_t::mapped_address, "mapped_address" },
        { stun_attribute_id_t::username, "username" },
        { stun_attribute_id_t::message_integrity, "message_integrity" },
        { stun_attribute_id_t::error_code, "error_code" },
        { stun_attribute_id_t::unknown, "unknown" },
        { stun_attribute_id_t::realm, "realm" },
        { stun_attribute_id_t::nonce, "nonce" },
        { stun_attribute_id_t::xor_mapped_address, "xor_mapped_address" },
        { stun_attribute_id_t::priority, "priority" },
        { stun_attribute_id_t::use_candidate, "use_candidate" },
        { stun_attribute_id_t::software, "software" },
        { stun_attribute_id_t::alternate_server, "alternate_server" },
        { stun_attribute_id_t::fingerprint, "fingerprint" },
        { stun_attribute_id_t::ice_controlled, "ice_controlled" },
        { stun_attribute_id_t::ice_controlling, "ice_controlling" },
    };

    auto it = conversion_table.find(attribute_id);
    return it != conversion_table.end()
            ? it->second
            : std::string{"undefined"};
}

// mapped address
stun_attribute_mapped_address_t::s_ptr_t stun_attribute_mapped_address_t::create(const socket_address_t &endpoint)
{
    return std::make_shared<stun_attribute_mapped_address_t>(endpoint);
}

stun_attribute_mapped_address_t::stun_attribute_mapped_address_t(const socket_address_t &endpoint)
     : stun_attribute_t(stun_attribute_id_t::mapped_address)
     , endpoint(endpoint)
{

}

std::string stun_attribute_mapped_address_t::to_string() const
{
    return stun_attribute_t::to_string().append(": address: ").append(endpoint.to_string());
}

// username
stun_attribute_username_t::s_ptr_t stun_attribute_username_t::create(const std::string &username)
{
    return std::make_shared<stun_attribute_username_t>(username);
}

stun_attribute_username_t::s_ptr_t stun_attribute_username_t::create(std::string &&username)
{
    return std::make_shared<stun_attribute_username_t>(std::move(username));
}

stun_attribute_username_t::stun_attribute_username_t(const std::string &username)
    : stun_attribute_t(stun_attribute_id_t::username)
    , username(username)
{

}

stun_attribute_username_t::stun_attribute_username_t(std::string &&username)
    : stun_attribute_t(stun_attribute_id_t::username)
    , username(std::move(username))
{

}

std::string stun_attribute_username_t::to_string() const
{
    return stun_attribute_t::to_string().append(": username: ").append(username);
}

// message intergity
stun_attribute_message_integrity_t::s_ptr_t stun_attribute_message_integrity_t::create(const stun_attribute_message_integrity_t::hmac_sha1_t &value)
{
    return std::make_shared<stun_attribute_message_integrity_t>(value);
}

stun_attribute_message_integrity_t::s_ptr_t stun_attribute_message_integrity_t::create(const void *data
                                                                                         , std::size_t size)
{
    return std::make_shared<stun_attribute_message_integrity_t>(data
                                                                , size);
}

stun_attribute_message_integrity_t::stun_attribute_message_integrity_t(const stun_attribute_message_integrity_t::hmac_sha1_t &value)
    : stun_attribute_t(stun_attribute_id_t::message_integrity)
    , value(value)
{

}

stun_attribute_message_integrity_t::stun_attribute_message_integrity_t(const void *data
                                                                       , std::size_t size)
    : stun_attribute_t(stun_attribute_id_t::message_integrity)
    , value(reinterpret_cast<const hmac_sha1_t::value_type*>(data)
            , reinterpret_cast<const hmac_sha1_t::value_type*>(data) + size)
{

}

std::string stun_attribute_message_integrity_t::to_string() const
{
    return stun_attribute_t::to_string().append(": hmac-sha1: ").append(utils::hex_to_string(value.data(), value.size()));
}

// error-code
stun_attribute_error_code_t::s_ptr_t stun_attribute_error_code_t::create(std::uint16_t error_code
                                                                         , const std::string& reason)
{
    return std::make_shared<stun_attribute_error_code_t>(error_code
                                                         , reason);
}

stun_attribute_error_code_t::stun_attribute_error_code_t(std::uint16_t error_code
                                                         , const std::string& reason)
    : stun_attribute_t(stun_attribute_id_t::error_code)
    , error_code(error_code)
    , reason(reason)
{

}

std::string stun_attribute_error_code_t::to_string() const
{
    return stun_attribute_t::to_string().append(": error-code: ").append(std::to_string(error_code));

}

uint8_t stun_attribute_error_code_t::error_class() const
{
    return (error_code / 100) & 0x07;
}

uint8_t stun_attribute_error_code_t::error_number() const
{
    return (error_code) % 100;
}

// unknown
stun_attribute_unknown_t::s_ptr_t stun_attribute_unknown_t::create(const void *data
                                                                     , std::size_t size)
{
    return std::make_shared<stun_attribute_unknown_t>(data
                                                      , size);
}

stun_attribute_unknown_t::s_ptr_t stun_attribute_unknown_t::create(const stun_attribute_unknown_t::unknown_data_t &unknown_data)
{
    return std::make_shared<stun_attribute_unknown_t>(unknown_data);
}

stun_attribute_unknown_t::stun_attribute_unknown_t(const stun_attribute_unknown_t::unknown_data_t &unknown_data)
    : stun_attribute_t(stun_attribute_id_t::unknown)
    , unknown_data(unknown_data)
{

}

stun_attribute_unknown_t::stun_attribute_unknown_t(const void *data
                                                   , std::size_t size)
    : stun_attribute_t(stun_attribute_id_t::unknown)
    , unknown_data(reinterpret_cast<const unknown_data_t::value_type*>(data)
                    , reinterpret_cast<const unknown_data_t::value_type*>(data) + size)
{

}

std::string stun_attribute_unknown_t::to_string() const
{
    return stun_attribute_t::to_string().append(": unknown-data: ").append(utils::hex_to_string(unknown_data.data(), unknown_data.size()));
}

// realm
stun_attribute_realm_t::s_ptr_t stun_attribute_realm_t::create(const std::string &value)
{
    return std::make_shared<stun_attribute_realm_t>(value);
}

stun_attribute_realm_t::stun_attribute_realm_t(const std::string &value)
    : stun_attribute_t(stun_attribute_id_t::realm)
    , value(value)
{

}

std::string stun_attribute_realm_t::to_string() const
{
    return stun_attribute_t::to_string().append(": value: ").append(value);
}

// nonce
stun_attribute_nonce_t::s_ptr_t stun_attribute_nonce_t::create(const std::string &value)
{
    return std::make_shared<stun_attribute_nonce_t>(value);
}

stun_attribute_nonce_t::stun_attribute_nonce_t(const std::string &value)
    : stun_attribute_t(stun_attribute_id_t::nonce)
    , value(value)
{

}

std::string stun_attribute_nonce_t::to_string() const
{
    return stun_attribute_t::to_string().append(": value: ").append(value);
}

// xor-mapped-address
stun_attribute_xor_mapped_address_t::s_ptr_t stun_attribute_xor_mapped_address_t::create(const socket_address_t &endpoint)
{
    return std::make_shared<stun_attribute_xor_mapped_address_t>(endpoint);
}

stun_attribute_xor_mapped_address_t::stun_attribute_xor_mapped_address_t(const socket_address_t &endpoint)
    : stun_attribute_t(stun_attribute_id_t::xor_mapped_address)
    , endpoint(endpoint)
{

}

std::string stun_attribute_xor_mapped_address_t::to_string() const
{
    return stun_attribute_t::to_string().append(": address: ").append(endpoint.to_string());
}

// priority
stun_attribute_priority_t::s_ptr_t stun_attribute_priority_t::create(const std::uint32_t &priority)
{
    return std::make_shared<stun_attribute_priority_t>(priority);
}

stun_attribute_priority_t::stun_attribute_priority_t(const std::uint32_t &priority)
    : stun_attribute_t(stun_attribute_id_t::priority)
    , priority(priority)
{

}

// use_candidate
std::string stun_attribute_priority_t::to_string() const
{
    return stun_attribute_t::to_string().append(": value: ").append(std::to_string(priority));
}

stun_attribute_use_candidate_t::s_ptr_t stun_attribute_use_candidate_t::create()
{
    return std::make_shared<stun_attribute_use_candidate_t>();
}

stun_attribute_use_candidate_t::stun_attribute_use_candidate_t()
    : stun_attribute_t(stun_attribute_id_t::use_candidate)
{

}

// software
stun_attribute_software_t::s_ptr_t stun_attribute_software_t::create()
{
    return std::make_shared<stun_attribute_software_t>();
}

stun_attribute_software_t::stun_attribute_software_t()
    : stun_attribute_t(stun_attribute_id_t::software)
{

}

// alternate-server
stun_attribute_alternate_server_t::s_ptr_t stun_attribute_alternate_server_t::create()
{
    return std::make_shared<stun_attribute_alternate_server_t>();
}

stun_attribute_alternate_server_t::stun_attribute_alternate_server_t()
    : stun_attribute_t(stun_attribute_id_t::alternate_server)
{

}

// fingerprint
stun_attribute_fingerprint_t::s_ptr_t stun_attribute_fingerprint_t::create(uint32_t value)
{
    return std::make_shared<stun_attribute_fingerprint_t>(value);
}

stun_attribute_fingerprint_t::stun_attribute_fingerprint_t(uint32_t value)
    : stun_attribute_t(stun_attribute_id_t::fingerprint)
    , value(value)
{

}

std::string stun_attribute_fingerprint_t::to_string() const
{
    return stun_attribute_t::to_string().append(": crc32: ").append(utils::hex_to_string(&value, sizeof(value)));
}

// ice-controlled
stun_attribute_ice_controlled_t::s_ptr_t stun_attribute_ice_controlled_t::create(uint64_t tie_breaker)
{
    return std::make_shared<stun_attribute_ice_controlled_t>(tie_breaker);
}

stun_attribute_ice_controlled_t::stun_attribute_ice_controlled_t(uint64_t tie_breaker)
    : stun_attribute_t(stun_attribute_id_t::ice_controlled)
    , tie_breaker(tie_breaker)
{

}

std::string stun_attribute_ice_controlled_t::to_string() const
{
    return stun_attribute_t::to_string().append(": tie_breaker: ").append(utils::hex_to_string(&tie_breaker, sizeof(tie_breaker)));
}

// ice-controlling
stun_attribute_ice_controlling_t::s_ptr_t stun_attribute_ice_controlling_t::create(uint64_t tie_breaker)
{
    return std::make_shared<stun_attribute_ice_controlling_t>(tie_breaker);
}

stun_attribute_ice_controlling_t::stun_attribute_ice_controlling_t(uint64_t tie_breaker)
    : stun_attribute_t(stun_attribute_id_t::ice_controlling)
    , tie_breaker(tie_breaker)
{

}

std::string stun_attribute_ice_controlling_t::to_string() const
{
    return stun_attribute_t::to_string().append(": tie_breaker: ").append(utils::hex_to_string(&tie_breaker, sizeof(tie_breaker)));
}



}
