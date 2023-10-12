#include "stun_message.h"
#include "stun_mapped_headers.h"
#include "utils/common_utils.h"
// #include "core/crypto_utils.h"
#include <algorithm>
#include <cstring>

namespace mpl::net
{

namespace
{
    bool parse_attributes(const stun_mapped_message_t& stun_message
                          , stun_attribute_t::w_ptr_list_t& attribute_list)
    {       
        bool has_fingerprint = false;
        bool has_message_integrity = false;

        for (const auto& a: stun_message.get_attributes())
        {
            auto attribute_id = a->header.get_attribute_id();
            if ((has_fingerprint)
                    || (has_message_integrity && attribute_id != stun_attribute_id_t::fingerprint))
            {
                return false;
            }

            auto payload_size = a->payload_size();
            const auto* payload = a->payload();

            stun_attribute_t::s_ptr_t attribute = nullptr;

            switch (attribute_id)
            {
                case stun_attribute_id_t::mapped_address:
                {
                    if (payload_size == 8)
                    {
                        auto ip_type = a->get_value<std::uint8_t>(1);
                        auto port = a->get_value<std::uint16_t>(2);
                        auto address = a->get_value<std::uint32_t>(4);
                        if (ip_type == 1) // ip_4
                        {
                            attribute = stun_attribute_mapped_address_t::create({ ip_address_t::ip4_address_t(address), port });
                        }
                    }
                }
                break;
                case stun_attribute_id_t::username:
                    if (payload_size > 0)
                    {
                        std::string username(reinterpret_cast<const char*>(payload), payload_size);
                        attribute = stun_attribute_username_t::create(username);
                    }
                break;
                case stun_attribute_id_t::message_integrity:
                    if (payload_size == 20)
                    {
                        attribute = stun_attribute_message_integrity_t::create(payload, payload_size);
                    }
                break;
                case stun_attribute_id_t::error_code:
                    if (payload_size >= 4)
                    {
                        auto error_code = static_cast<std::uint16_t>(a->get_value<std::uint8_t>(2)) * 100
                                + static_cast<std::uint16_t>(a->get_value<std::uint8_t>(3));
                        std::string reason = {};

                        if (auto len = payload_size - 4)
                        {
                            reason.assign(reinterpret_cast<const char*>(payload) + 4
                                          , len);
                        }
                        attribute = stun_attribute_error_code_t::create(error_code
                                                                        , reason);
                    }
                break;
                case stun_attribute_id_t::unknown:
                    if (payload_size > 0)
                    {
                        attribute = stun_attribute_unknown_t::create(payload, payload_size);
                    }
                break;
                case stun_attribute_id_t::realm:
                    if (payload_size > 0)
                    {
                        std::string realm(reinterpret_cast<const char*>(payload), payload_size);
                        attribute = stun_attribute_realm_t::create(realm);
                    }
                break;
                case stun_attribute_id_t::nonce:
                    if (payload_size > 0)
                    {
                        std::string nonce(reinterpret_cast<const char*>(payload), payload_size);
                        attribute = stun_attribute_nonce_t::create(nonce);
                    }
                break;
                case stun_attribute_id_t::xor_mapped_address:
                    if (payload_size >= 7)
                    {
                        auto ip_type = a->get_value<std::uint8_t>(1);
                        auto port = a->get_value<std::uint16_t>(2);
                        auto address = a->get_value<std::uint32_t>(4);

                        if (ip_type == 1) // only ip_4
                        {
                            const auto xor_mapped = stun_message.payload(-16);
                            reinterpret_cast<std::uint8_t*>(&port)[1] ^= xor_mapped[0];
                            reinterpret_cast<std::uint8_t*>(&port)[0] ^= xor_mapped[1];
                            reinterpret_cast<std::uint8_t*>(&address)[3] ^= xor_mapped[0];
                            reinterpret_cast<std::uint8_t*>(&address)[2] ^= xor_mapped[1];
                            reinterpret_cast<std::uint8_t*>(&address)[1] ^= xor_mapped[2];
                            reinterpret_cast<std::uint8_t*>(&address)[0] ^= xor_mapped[3];

                            attribute = stun_attribute_xor_mapped_address_t::create({ ip_address_t::ip4_address_t(address), port });
                        }
                    }
                break;
                case stun_attribute_id_t::priority:
                    if (payload_size == 4)
                    {
                        auto priority = a->get_value<std::uint32_t>();
                        attribute = stun_attribute_priority_t::create(priority);
                    }
                break;
                case stun_attribute_id_t::use_candidate:
                    if (payload_size == 0)
                    {
                        attribute = stun_attribute_use_candidate_t::create();
                    }
                break;
                case stun_attribute_id_t::software:
                    attribute = stun_attribute_software_t::create();
                break;
                case stun_attribute_id_t::alternate_server:
                    attribute = stun_attribute_software_t::create();
                break;
                case stun_attribute_id_t::fingerprint:
                    has_fingerprint = true;
                    if (payload_size == 4)
                    {
                        auto crc32 = a->get_value<std::uint32_t>();
                        auto crc_size = reinterpret_cast<std::size_t>(a) - reinterpret_cast<std::size_t>(&stun_message);
                        auto calc_crc32 = utils::crypto::hash<utils::crypto::hash_method_t::crc32, std::uint32_t>(&stun_message, crc_size) ^ stun_fingerprint_xor_value;

                        if (crc32 == calc_crc32)
                        {
                            attribute = stun_attribute_fingerprint_t::create(crc32);
                        }
                    }
                break;
                case stun_attribute_id_t::ice_controlled:
                    if (payload_size == 8)
                    {
                        auto tie_breaker = a->get_value<std::uint64_t>();
                        attribute = stun_attribute_ice_controlled_t::create(tie_breaker);
                    }
                break;
                case stun_attribute_id_t::ice_controlling:
                    if (payload_size == 8)
                    {
                        auto tie_breaker = a->get_value<std::uint64_t>();
                        attribute = stun_attribute_ice_controlling_t::create(tie_breaker);
                    }
                break;
                default:
                    continue;
            }

            if (attribute != nullptr)
            {
                attribute_list.emplace_back(std::move(attribute));
            }
            else
            {
                return false;
            }

        }

        return true;
    }

    std::size_t pack_attributes(raw_array_t& packet_buffer
                                , const stun_attribute_t::w_ptr_list_t& attributes
                                , const std::string& password)
    {
        bool has_message_integrity = false;

        raw_array_packer packer(packet_buffer);

        auto saved_size = packet_buffer.size();

        for (const auto& a : attributes)
        {
            if (has_message_integrity)
            {
                break;
            }

            std::uint16_t attribute_id = utils::endian::big_endian_convert(static_cast<std::uint16_t>(a->attribute_id));

            std::uint16_t payload_size = 0;

            switch(a->attribute_id)
            {
                case stun_attribute_id_t::mapped_address:
                {
                    const stun_attribute_mapped_address_t& mapped_address_attribute = static_cast<const stun_attribute_mapped_address_t&>(*a);

                    if (mapped_address_attribute.endpoint.is_valid())
                    {
                        payload_size = 8;
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .padding(1)
                              .append<std::uint8_t>(1)
                              .append<std::uint16_t>(utils::endian::big_endian_convert<std::uint16_t>(mapped_address_attribute.endpoint.port))
                              .append<std::uint32_t>(utils::endian::big_endian_convert<std::uint32_t>(mapped_address_attribute.endpoint.address.ip4_address.address));
                    }
                }
                break;
                case stun_attribute_id_t::username:
                {
                    const stun_attribute_username_t& username_attribute = static_cast<const stun_attribute_username_t&>(*a);
                    payload_size = username_attribute.username.length();
                    if (payload_size > 0)
                    {
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .append(username_attribute.username.data()
                                      , payload_size);

                    }

                }
                break;
                case stun_attribute_id_t::message_integrity:
                {
                    continue;
                    /*has_message_integrity = true;
                    const stun_attribute_message_integrity_t& message_integrity_attribute = static_cast<const stun_attribute_message_integrity_t&>(*a);
                    payload_size = message_integrity_attribute.value.size();
                    if (payload_size > 0)
                    {
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .append(message_integrity_attribute.value.data()
                                      , payload_size);
                    }*/
                }
                break;
                case stun_attribute_id_t::error_code:
                {
                    const stun_attribute_error_code_t& error_code_attribute = static_cast<const stun_attribute_error_code_t&>(*a);
                    payload_size = 4 + error_code_attribute.reason.size();

                    packer.append(attribute_id)
                          .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                          .padding(2)
                          .append(error_code_attribute.error_class())
                          .append(error_code_attribute.error_number());

                    if (!error_code_attribute.reason.empty())
                    {
                        packer.append(error_code_attribute.reason.data()
                                      , error_code_attribute.reason.size());
                    }
                }
                break;
                case stun_attribute_id_t::unknown:
                {
                    const stun_attribute_unknown_t& unknown_attribute = static_cast<const stun_attribute_unknown_t&>(*a);
                    payload_size = unknown_attribute.unknown_data.size();
                    if (payload_size > 0)
                    {
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .append(unknown_attribute.unknown_data.data()
                                      , payload_size);
                    }
                }
                break;
                case stun_attribute_id_t::realm:
                {
                    const stun_attribute_realm_t& realm_attribute = static_cast<const stun_attribute_realm_t&>(*a);
                    payload_size = realm_attribute.value.length();
                    if (payload_size > 0)
                    {
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .append(realm_attribute.value.data()
                                      , payload_size);
                    }
                }
                break;
                case stun_attribute_id_t::nonce:
                {
                    const stun_attribute_nonce_t&  nonce_attribute = static_cast<const stun_attribute_nonce_t&>(*a);
                    payload_size = nonce_attribute.value.length();
                    if (payload_size > 0)
                    {
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .append(nonce_attribute.value.data()
                                      , payload_size);
                    }
                }
                break;
                case stun_attribute_id_t::xor_mapped_address:
                {
                    const stun_attribute_xor_mapped_address_t& xor_mapped_address_attribute = static_cast<const stun_attribute_xor_mapped_address_t&>(*a);
                    std::uint32_t address = xor_mapped_address_attribute.endpoint.address.ip4_address.address;

                    if (xor_mapped_address_attribute.endpoint.is_valid())
                    {
                        auto port = xor_mapped_address_attribute.endpoint.port;

                        const auto xor_mapped = reinterpret_cast<stun_mapped_message_t*>(packet_buffer.data())->payload(-16);
                        reinterpret_cast<std::uint8_t*>(&port)[1] ^= xor_mapped[0];
                        reinterpret_cast<std::uint8_t*>(&port)[0] ^= xor_mapped[1];
                        reinterpret_cast<std::uint8_t*>(&address)[3] ^= xor_mapped[0];
                        reinterpret_cast<std::uint8_t*>(&address)[2] ^= xor_mapped[1];
                        reinterpret_cast<std::uint8_t*>(&address)[1] ^= xor_mapped[2];
                        reinterpret_cast<std::uint8_t*>(&address)[0] ^= xor_mapped[3];
                        payload_size = 8;
                        packer.append(attribute_id)
                              .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                              .padding(1)
                              .append<std::uint8_t>(1)
                              .append<std::uint16_t>(utils::endian::big_endian_convert<std::uint16_t>(port))
                              .append<std::uint32_t>(utils::endian::big_endian_convert<std::uint32_t>(address));
                    }
                }
                break;
                case stun_attribute_id_t::priority:
                {
                    const stun_attribute_priority_t&  priority_attribute = static_cast<const stun_attribute_priority_t&>(*a);
                    payload_size = 4;
                    packer.append(attribute_id)
                          .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                          .append(utils::endian::big_endian_convert<std::uint32_t>(priority_attribute.priority));
                }
                break;
                case stun_attribute_id_t::use_candidate:
                {
                    payload_size = 0;
                    packer.append(attribute_id)
                          .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size));
                }
                break;
                case stun_attribute_id_t::software:
                {
                    continue;
                }
                break;
                case stun_attribute_id_t::alternate_server:
                {
                    continue;
                }
                break;
                case stun_attribute_id_t::fingerprint:
                {
                    continue;
                }
                break;
                case stun_attribute_id_t::ice_controlled:
                {
                    const stun_attribute_ice_controlled_t& ice_controlled_attribute = static_cast<const stun_attribute_ice_controlled_t&>(*a);
                    payload_size = 8;
                    packer.append(attribute_id)
                          .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                          .append(utils::endian::big_endian_convert<std::uint64_t>(ice_controlled_attribute.tie_breaker));
                }
                break;
                case stun_attribute_id_t::ice_controlling:
                {
                    const stun_attribute_ice_controlling_t& ice_controlling_attribute = static_cast<const stun_attribute_ice_controlling_t&>(*a);
                    payload_size = 8;
                    packer.append(attribute_id)
                          .append(utils::endian::big_endian_convert<std::uint16_t>(payload_size))
                          .append(utils::endian::big_endian_convert<std::uint64_t>(ice_controlling_attribute.tie_breaker));
                }
                break;
                default:
                    continue;
            }

            if (payload_size % stun_align_size > 0)
            {
                packer.padding(stun_align_size - payload_size % stun_align_size);
            }
        }

        auto payload_size = packet_buffer.size() - saved_size;

        if (payload_size > 0)
        {
            if (!password.empty())
            {
                auto hash_size = payload_size + sizeof(stun_mapped_header_t);
                payload_size += 24;

                reinterpret_cast<stun_mapped_message_t*>(packet_buffer.data())->set_payload_size(payload_size);
                auto calc_hash = utils::crypto::hash_hmac_sha1(password
                                                               , packet_buffer.data()
                                                               , hash_size);
                packer.append(utils::endian::big_endian_convert<std::uint16_t>(static_cast<std::uint16_t>(stun_attribute_id_t::message_integrity)))
                      .append(utils::endian::big_endian_convert<std::uint16_t>(calc_hash.size()))
                      .append(calc_hash.data()
                              , calc_hash.size());

                reinterpret_cast<stun_mapped_message_t*>(packet_buffer.data())->set_payload_size(payload_size + 8);
                auto calc_crc32 = utils::crypto::hash<utils::crypto::hash_method_t::crc32, std::uint32_t>(packet_buffer.data(), packet_buffer.size()) ^ stun_fingerprint_xor_value;

                packer.append(utils::endian::big_endian_convert<std::uint16_t>(static_cast<std::uint16_t>(stun_attribute_id_t::fingerprint)))
                      .append(utils::endian::big_endian_convert<std::uint16_t>(4))
                      .append(utils::endian::big_endian_convert<std::uint32_t>(calc_crc32));

            }
            else
            {
                reinterpret_cast<stun_mapped_message_t*>(packet_buffer.data())->set_payload_size(payload_size);
            }
        }

        return packet_buffer.size() - saved_size;
    }

}

bool stun_message_t::parse(const i_data_object &parse_data
                           , stun_message_t &message)
{
    const stun_mapped_message_t& stun_message = *static_cast<const stun_mapped_message_t*>(parse_data.data());
    if (stun_message.is_valid(parse_data.size()))
    {
        message.message_class = stun_message.header.get_class();
        message.method = stun_message.header.get_method();
        std::memcpy(message.transaction_id.data()
                    , stun_message.header.transaction_id
                    , sizeof(stun_message.header.transaction_id));


        if (parse_attributes(stun_message
                             , message.attributes))
        {
            return message.is_valid();
        }
    }
    return false;
}


stun_transaction_id_t stun_message_t::generate_transaction_id()
{
    stun_transaction_id_t transaction;
    for (auto& c: transaction)
    {
        c = utils::random<stun_transaction_id_t::value_type>();
    }
    return transaction;
}

stun_message_t::stun_message_t(stun_message_class_t message_class
                                     , stun_method_t method
                                     , const stun_transaction_id_t &transaction_id
                                     , const stun_attribute_t::w_ptr_list_t& attributes)
    : message_class(message_class)
    , method(method)
    , transaction_id(transaction_id)
    , attributes(attributes)
{

}

stun_attribute_t::s_ptr_t stun_message_t::get_attribute(stun_attribute_id_t attribute_id)
{
    auto it = std::find_if(attributes.begin()
                           , attributes.end()
                           , [&attribute_id](const auto& a) { return a->attribute_id == attribute_id;} );

    return it != attributes.end()
            ? *it
            : nullptr;
}

bool stun_message_t::is_valid() const
{
    return message_class != stun_message_class_t::undefined
            && method != stun_method_t::undefined;
}

void stun_message_t::reset()
{
    message_class = stun_message_class_t::undefined;
    method = stun_method_t::undefined;
    transaction_id.fill(0);
    attributes.clear();
}

smart_buffer stun_message_t::build_packet(const std::string& password) const
{
    raw_array_t    buffer(sizeof(stun_mapped_header_t));

    reinterpret_cast<stun_mapped_header_t*>(buffer.data())->set_class(message_class);
    reinterpret_cast<stun_mapped_header_t*>(buffer.data())->set_method(method);
    reinterpret_cast<stun_mapped_header_t*>(buffer.data())->cookie = stun_message_cookie;
    std::memcpy(reinterpret_cast<stun_mapped_header_t*>(buffer.data())->transaction_id
                , transaction_id.data()
                , transaction_id.size());

    pack_attributes(buffer
                    , attributes
                    , password);

    return smart_buffer(std::move(buffer));
}

std::string stun_message_t::to_string() const
{
    static const std::string class_name_table[] =
    {
        "undefined",
        "request",
        "indication",
        "success_response",
        "error_response"
    };

    return std::string("class: ").append(class_name_table[static_cast<std::int32_t>(message_class) + 1])
            .append(", method: ").append(method == stun_method_t::binding ? std::string{"binding"} : std::string{"undefined"})
            .append(", transaction_id: ").append(utils::string::hex::to_string(transaction_id.data(), transaction_id.size()))
            .append(", attributes: ").append(std::to_string(attributes.size()));
}

}
