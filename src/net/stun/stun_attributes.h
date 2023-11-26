#ifndef MPL_NET_STUN_ATTRIBUTE_H
#define MPL_NET_STUN_ATTRIBUTE_H

#include "stun_types.h"
#include "net/socket/socket_endpoint.h"
#include <memory>
#include <vector>

namespace mpl::net
{

struct stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_t>;
    using s_ptr_list_t = std::vector<s_ptr_t>;

    const stun_attribute_id_t attribute_id;
protected:
    stun_attribute_t(stun_attribute_id_t attribute_id);

public:
    virtual std::string to_string() const;

    template<typename T>
    const T& cast() const { return static_cast<const T&>(*this); }

    template<typename T>
    T& cast() { return static_cast<T&>(*this); }

};

// mapped-address
struct stun_attribute_mapped_address_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_mapped_address_t>;

    socket_address_t  endpoint;

    static s_ptr_t create(const socket_address_t& endpoint);
    stun_attribute_mapped_address_t(const socket_address_t& endpoint);
    std::string to_string() const override;
};

// username
struct stun_attribute_username_t : public stun_attribute_t
{
    std::string     username;
    using s_ptr_t = std::shared_ptr<stun_attribute_username_t>;

    static s_ptr_t create(const std::string& username);
    static s_ptr_t create(std::string&& username);
    stun_attribute_username_t(const std::string& username);
    stun_attribute_username_t(std::string&& username);
    std::string to_string() const override;
};

// message-integrity
struct stun_attribute_message_integrity_t : public stun_attribute_t
{
    using hmac_sha1_t = std::vector<std::uint8_t>;

    using s_ptr_t = std::shared_ptr<stun_attribute_message_integrity_t>;

    hmac_sha1_t     value;

    static s_ptr_t create(const hmac_sha1_t& value);
    static s_ptr_t create(const void* data
                            , std::size_t size);
    stun_attribute_message_integrity_t(const hmac_sha1_t& value);
    stun_attribute_message_integrity_t(const void* data
                                       , std::size_t size);
    std::string to_string() const override;
};

// error-code
struct stun_attribute_error_code_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_error_code_t>;

    std::uint16_t   error_code;
    std::string     reason;

    static s_ptr_t create(std::uint16_t
                           , const std::string& reason = {});
    stun_attribute_error_code_t(std::uint16_t
                                , const std::string& reason = {});
    std::string to_string() const override;

    std::uint8_t error_class() const;
    std::uint8_t error_number() const;

};

// unknown
struct stun_attribute_unknown_t : public stun_attribute_t
{
    using unknown_data_t = std::vector<std::uint8_t>;
    using s_ptr_t = std::shared_ptr<stun_attribute_unknown_t>;

    unknown_data_t  unknown_data;

    static s_ptr_t create(const void* data
                           , std::size_t size);
    static s_ptr_t create(const unknown_data_t& unknown_data);

    stun_attribute_unknown_t(const unknown_data_t& unknown_data);
    stun_attribute_unknown_t(const void* data
                             , std::size_t size);

    std::string to_string() const override;
};

// realm
struct stun_attribute_realm_t : public stun_attribute_t
{
    std::string     value;

    using s_ptr_t = std::shared_ptr<stun_attribute_realm_t>;

    static s_ptr_t create(const std::string& value);

    stun_attribute_realm_t(const std::string& value);

    std::string to_string() const override;
};

// nonce
struct stun_attribute_nonce_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_nonce_t>;

    std::string     value;

    static s_ptr_t create(const std::string& value);

    stun_attribute_nonce_t(const std::string& value);

    std::string to_string() const override;
};

// xor-mapped-address
struct stun_attribute_xor_mapped_address_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_xor_mapped_address_t>;

    socket_address_t  endpoint;

    static s_ptr_t create(const socket_address_t& endpoint);
    stun_attribute_xor_mapped_address_t(const socket_address_t& endpoint);

    std::string to_string() const override;
};

// priority
struct stun_attribute_priority_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_priority_t>;

    std::uint32_t   priority;

    static s_ptr_t create(const std::uint32_t& priority);
    stun_attribute_priority_t(const std::uint32_t& priority);


    std::string to_string() const override;
};

// use-candidate
struct stun_attribute_use_candidate_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_use_candidate_t>;

    static s_ptr_t create();
    stun_attribute_use_candidate_t();
};

// software
struct stun_attribute_software_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_software_t>;

    static s_ptr_t create();
    stun_attribute_software_t();
};

// alternate-server
struct stun_attribute_alternate_server_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_alternate_server_t>;

    static s_ptr_t create();
    stun_attribute_alternate_server_t();
};

// fingerprint
struct stun_attribute_fingerprint_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_fingerprint_t>;

    std::uint32_t   value;

    static s_ptr_t create(std::uint32_t value);
    stun_attribute_fingerprint_t(std::uint32_t value);

    std::string to_string() const override;
};

// ice-controlled
struct stun_attribute_ice_controlled_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_ice_controlled_t>;

    std::uint64_t   tie_breaker;

    static s_ptr_t create(std::uint64_t tie_breaker);
    stun_attribute_ice_controlled_t(std::uint64_t tie_breaker);

    std::string to_string() const override;
};

// ice-controlling
struct stun_attribute_ice_controlling_t : public stun_attribute_t
{
    using s_ptr_t = std::shared_ptr<stun_attribute_ice_controlling_t>;

    std::uint64_t   tie_breaker;

    static s_ptr_t create(std::uint64_t tie_breaker);
    stun_attribute_ice_controlling_t(std::uint64_t tie_breaker);

    std::string to_string() const override;
};


}

#endif // MPL_NET_STUN_ATTRIBUTE_H
