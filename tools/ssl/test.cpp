#include "test.h"
#include "ssl_utils.h"
#include "ssl_types.h"
#include "openssl/ssl.h"
#include "bio_context.h"
#include "bio_config.h"
#include "ssl_context_config.h"
#include "ssl_context.h"

#include "ssl_manager_config.h"
#include "ssl_connection_config.h"
#include "ssl_adapter.h"
#include "const_ssl_message.h"
#include "dynamic_ssl_message.h"
#include "srtp_key_info.h"
#include "i_ssl_message_sink.h"
#include "ssl_session_manager.h"

#include "tools/utils/string_base.h"

#include <vector>
#include <iostream>
#include <thread>
#include <queue>


namespace pt::ssl
{

void test1()
{
    const std::string cert_string =
        "-----BEGIN CERTIFICATE-----\n"
        "MIIDaDCCAlCgAwIBAgIJAO8vBu8i8exWMA0GCSqGSIb3DQEBCwUAMEkxCzAJBgNV\n"
        "BAYTAlVTMQswCQYDVQQIDAJDQTEtMCsGA1UEBwwkTG9zIEFuZ2VsZXNPPUJlYXN0\n"
        "Q049d3d3LmV4YW1wbGUuY29tMB4XDTE3MDUwMzE4MzkxMloXDTQ0MDkxODE4Mzkx\n"
        "MlowSTELMAkGA1UEBhMCVVMxCzAJBgNVBAgMAkNBMS0wKwYDVQQHDCRMb3MgQW5n\n"
        "ZWxlc089QmVhc3RDTj13d3cuZXhhbXBsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUA\n"
        "A4IBDwAwggEKAoIBAQDJ7BRKFO8fqmsEXw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcF\n"
        "xqGitbnLIrOgiJpRAPLy5MNcAXE1strVGfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7b\n"
        "Fu8TsCzO6XrxpnVtWk506YZ7ToTa5UjHfBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO\n"
        "9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wWKIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBp\n"
        "yY8anC8u4LPbmgW0/U31PH0rRVfGcBbZsAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrv\n"
        "enu2tOK9Qx6GEzXh3sekZkxcgh+NlIxCNxu//Dk9AgMBAAGjUzBRMB0GA1UdDgQW\n"
        "BBTZh0N9Ne1OD7GBGJYz4PNESHuXezAfBgNVHSMEGDAWgBTZh0N9Ne1OD7GBGJYz\n"
        "4PNESHuXezAPBgNVHRMBAf8EBTADAQH/MA0GCSqGSIb3DQEBCwUAA4IBAQCmTJVT\n"
        "LH5Cru1vXtzb3N9dyolcVH82xFVwPewArchgq+CEkajOU9bnzCqvhM4CryBb4cUs\n"
        "gqXWp85hAh55uBOqXb2yyESEleMCJEiVTwm/m26FdONvEGptsiCmF5Gxi0YRtn8N\n"
        "V+KhrQaAyLrLdPYI7TrwAOisq2I1cD0mt+xgwuv/654Rl3IhOMx+fKWKJ9qLAiaE\n"
        "fQyshjlPP9mYVxWOxqctUdQ8UnsUKKGEUcVrA08i1OAnVKlPFjKBvk+r7jpsTPcr\n"
        "9pWXTO9JrYMML7d+XRSZA1n3856OqZDX4403+9FnXCvfcLZLLKTBvwwFgEFGpzjK\n"
        "UEVbkhd5qstF6qWK\n"
        "-----END CERTIFICATE-----\n";

    const std::string key_string =
        "-----BEGIN PRIVATE KEY-----\n"
        "MIIEvgIBADANBgkqhkiG9w0BAQEFAASCBKgwggSkAgEAAoIBAQDJ7BRKFO8fqmsE\n"
        "Xw8v9YOVXyrQVsVbjSSGEs4Vzs4cJgcFxqGitbnLIrOgiJpRAPLy5MNcAXE1strV\n"
        "GfdEf7xMYSZ/4wOrxUyVw/Ltgsft8m7bFu8TsCzO6XrxpnVtWk506YZ7ToTa5UjH\n"
        "fBi2+pWTxbpN12UhiZNUcrRsqTFW+6fO9d7xm5wlaZG8cMdg0cO1bhkz45JSl3wW\n"
        "KIES7t3EfKePZbNlQ5hPy7Pd5JTmdGBpyY8anC8u4LPbmgW0/U31PH0rRVfGcBbZ\n"
        "sAoQw5Tc5dnb6N2GEIbq3ehSfdDHGnrvenu2tOK9Qx6GEzXh3sekZkxcgh+NlIxC\n"
        "Nxu//Dk9AgMBAAECggEBAK1gV8uETg4SdfE67f9v/5uyK0DYQH1ro4C7hNiUycTB\n"
        "oiYDd6YOA4m4MiQVJuuGtRR5+IR3eI1zFRMFSJs4UqYChNwqQGys7CVsKpplQOW+\n"
        "1BCqkH2HN/Ix5662Dv3mHJemLCKUON77IJKoq0/xuZ04mc9csykox6grFWB3pjXY\n"
        "OEn9U8pt5KNldWfpfAZ7xu9WfyvthGXlhfwKEetOuHfAQv7FF6s25UIEU6Hmnwp9\n"
        "VmYp2twfMGdztz/gfFjKOGxf92RG+FMSkyAPq/vhyB7oQWxa+vdBn6BSdsfn27Qs\n"
        "bTvXrGe4FYcbuw4WkAKTljZX7TUegkXiwFoSps0jegECgYEA7o5AcRTZVUmmSs8W\n"
        "PUHn89UEuDAMFVk7grG1bg8exLQSpugCykcqXt1WNrqB7x6nB+dbVANWNhSmhgCg\n"
        "VrV941vbx8ketqZ9YInSbGPWIU/tss3r8Yx2Ct3mQpvpGC6iGHzEc/NHJP8Efvh/\n"
        "CcUWmLjLGJYYeP5oNu5cncC3fXUCgYEA2LANATm0A6sFVGe3sSLO9un1brA4zlZE\n"
        "Hjd3KOZnMPt73B426qUOcw5B2wIS8GJsUES0P94pKg83oyzmoUV9vJpJLjHA4qmL\n"
        "CDAd6CjAmE5ea4dFdZwDDS8F9FntJMdPQJA9vq+JaeS+k7ds3+7oiNe+RUIHR1Sz\n"
        "VEAKh3Xw66kCgYB7KO/2Mchesu5qku2tZJhHF4QfP5cNcos511uO3bmJ3ln+16uR\n"
        "GRqz7Vu0V6f7dvzPJM/O2QYqV5D9f9dHzN2YgvU9+QSlUeFK9PyxPv3vJt/WP1//\n"
        "zf+nbpaRbwLxnCnNsKSQJFpnrE166/pSZfFbmZQpNlyeIuJU8czZGQTifQKBgHXe\n"
        "/pQGEZhVNab+bHwdFTxXdDzr+1qyrodJYLaM7uFES9InVXQ6qSuJO+WosSi2QXlA\n"
        "hlSfwwCwGnHXAPYFWSp5Owm34tbpp0mi8wHQ+UNgjhgsE2qwnTBUvgZ3zHpPORtD\n"
        "23KZBkTmO40bIEyIJ1IZGdWO32q79nkEBTY+v/lRAoGBAI1rbouFYPBrTYQ9kcjt\n"
        "1yfu4JF5MvO9JrHQ9tOwkqDmNCWx9xWXbgydsn/eFtuUMULWsG3lNjfst/Esb8ch\n"
        "k5cZd6pdJZa4/vhEwrYYSuEjMCnRb0lUsm7TsHxQrUd6Fi/mUuFU/haC0o0chLq7\n"
        "pVOUFq5mW8p0zbtfHbjkgxyF\n"
        "-----END PRIVATE KEY-----\n";

    auto cert_path = "/home/user/test_cert.pem";
    auto key_path = "/home/user/test_pkey.pem";

    bio_context bio_cert{ };
    bio_context bio_pkey{  };

    bio_cert.read_from_file(cert_path);
    bio_pkey.read_from_file(key_path);

    auto cert = bio_cert.read_x509();
    auto pkey = bio_pkey.read_evp_pkey();

    bio_context bio_cert_save{  };
    bio_context bio_pkey_save{  };

    auto r1 = bio_cert_save.write_x509(cert);
    auto r2 = bio_pkey_save.write_evp_pkey(pkey);

    bio_cert_save.write_to_file("/home/user/test_cert_2.pem");
    bio_pkey_save.write_to_file("/home/user/test_pkey_2.pem");

    return;
}

void test2()
{
    constexpr auto cert_path = "/home/user/test_cert.pem";

    bio_config_t config { };
    bio_context bio;

    auto result = bio.read_from_file(cert_path);
    if (result)
    {
        auto read_pending = bio.read_pending();
        auto write_pending = bio.write_pending();
        auto read_count = bio.read_count();
        auto write_count = bio.write_count();

        auto n_bio = bio.native_handle().get();

        auto len = BIO_read(n_bio, nullptr, 0);

        auto need_read = 10000;
        if (need_read > 0)
        {
            std::vector<std::uint8_t> buffer(need_read);
            auto size = bio.read(buffer.data(), buffer.size());
            if (size)
            {
                return;
            }
        }
    }

    return;
}


void test3()
{

    ssl_context_config_t config;
    config.method = ssl_method_t::dtls_method;
    config.set_flag(ssl_options_flags_t::cipter_server_preference, true)
            .set_flag(ssl_options_flags_t::no_ticket, true)
            .set_flag(ssl_options_flags_t::no_query_mtu, true)
            .set_flag(ssl_session_cache_flags_t::off, true)
            .set_flag(ssl_verify_flags_t::peer, true)
            .set_flag(ssl_verify_flags_t::fail_if_no_peer_cert, true);

    ssl_context context(config);

    if (auto ssl = context.create_ssl())
    {
        std::cout << "ssl create complete" << std::endl;
    }


    return;
}

void test4()
{
    /*
    {

        ssl_context_config_t config;
        config.method = ssl_method_t::dtls_method;
        config.set_flag(ssl_options_flags_t::cipter_server_preference, true)
                .set_flag(ssl_options_flags_t::no_ticket, true)
                .set_flag(ssl_options_flags_t::no_query_mtu, true)
                .set_flag(ssl_session_cache_flags_t::off, true)
                .set_flag(ssl_verify_flags_t::peer, true)
                .set_flag(ssl_verify_flags_t::fail_if_no_peer_cert, true);
        ssl_context ctx(config);

        bio_context bio1;
        bio_context bio2;
        ssl_adapter ssl(ctx.native_handle(), bio1.native_handle(), bio2.native_handle());
    }
    {
        using message_record_t = std::pair<ssl_role_t, dynamic_ssl_message>;
        std::queue<message_record_t> message_queue;

        static const std::string state_table[] =
        {
            "ready",
            "prepare",
            "handshaking",
            "done",
            "closed",
            "failed"
        };

        static const std::string ciper_list = ssl_manager_config_t::default_ciper_list;
        constexpr std::uint32_t default_mtu = 1500;

        ssl_manager_config_t    server_config;
        server_config.context_params.method = ssl_method_t::dtls_method;
        server_config.subject = "server01";
        server_config.context_params.set_flag(ssl_options_flags_t::cipter_server_preference, true)
                                            .set_flag(ssl_options_flags_t::no_ticket, true)
                                            .set_flag(ssl_options_flags_t::no_query_mtu, true)
                                            .set_flag(ssl_session_cache_flags_t::off, true)
                                            .set_flag(ssl_verify_flags_t::peer, true)
                                            .set_flag(ssl_verify_flags_t::fail_if_no_peer_cert, true);

        server_config.ciper_list = ciper_list;
        server_config.srtp_profiles = ssl_manager_config_t::default_srtp_profiles;

        ssl_manager_config_t    client_config = server_config;
        client_config.subject = "client01";

        auto on_state = [&](const std::string& label, ssl_handshake_state_t state)
        {
            std::cout << label << ": change state: " << state_table[static_cast<std::int32_t>(state)] << std::endl;
        };

        auto on_message = [&](const std::string& label, const i_ssl_message& message)
        {
            std::cout << label << ": on message(" << static_cast<std::int32_t>(message.type()) << "): "
                      << message.size() << " bytes" << std::endl;
            return ssl_io_result_t::ok;
        };

        auto on_key = [&](const std::string& label, const srtp_key_info_t& client_key, const srtp_key_info_t& server_key)
        {
            std::cout << label << ": on key: client[" << to_string<srtp_profile_id_t>(client_key.profile_id) << "]: " << portable::hex_dump(client_key.key.data(), client_key.key.size())
                      << ", server[" << to_string<srtp_profile_id_t>(server_key.profile_id) << "]: " << portable::hex_dump(server_key.key.data(), server_key.key.size()) << std::endl;
            return ssl_io_result_t::ok;
        };

        auto on_error = [&](const std::string& label, ssl_alert_type_t alert_type, const std::string& reason)
        {
            std::cout << label << ": on error(" << std::int32_t(alert_type) << "): " << reason << std::endl;
            return ssl_io_result_t::ok;
        };

        i_ssl_session::u_ptr_t server_connection = nullptr;
        i_ssl_session::u_ptr_t client_connection = nullptr;

        auto on_server_state = [&](ssl_handshake_state_t state)
        {
            on_state("server", state);
        };

        auto on_client_state = [&](ssl_handshake_state_t state)
        {
            on_state("client", state);
        };

        auto on_server_message = [&](const i_ssl_message& message)
        {
            on_message("server", message);
            message_queue.emplace(ssl_role_t::client, message);
            return ssl_io_result_t::ok;
            // return client_connection->send_message(message);
        };

        auto on_client_message = [&](const i_ssl_message& message)
        {
            on_message("client", message);
            message_queue.emplace(ssl_role_t::server, message);
            return ssl_io_result_t::ok;
            //return server_connection->send_message(message);
        };

        auto on_server_key = [&](const srtp_key_info_t& client_key, const srtp_key_info_t& server_key)
        {
            on_key("server", client_key, server_key);
        };

        auto on_client_key = [&](const srtp_key_info_t& client_key, const srtp_key_info_t& server_key)
        {
            on_key("client", client_key, server_key);
        };

        auto on_client_error = [&](ssl_alert_type_t alert_type, const std::string& reason)
        {
            on_error("client", alert_type, reason);
        };

        auto on_server_error = [&](ssl_alert_type_t alert_type, const std::string& reason)
        {
            on_error("server", alert_type, reason);
        };

        auto server_observer = custom_ssl_connection_observer::create(on_server_message
                                                                      , on_server_state
                                                                      , on_server_key
                                                                      , on_server_error);

        auto client_observer = custom_ssl_connection_observer::create(on_client_message
                                                                      , on_client_state
                                                                      , on_client_key
                                                                      , on_client_error);

        ssl_connection_config_t server_connection_config(ssl_role_t::server, default_mtu);
        ssl_connection_config_t client_connection_config(ssl_role_t::client, default_mtu);

        ssl_manager server_manager(server_config);
        ssl_manager client_manager(client_config);

        server_connection = server_manager.create_connection(server_connection_config
                                                             , server_observer.get());
        client_connection = client_manager.create_connection(client_connection_config
                                                             , client_observer.get());



        server_connection->control(ssl_control_id_t::handshake);
        client_connection->control(ssl_control_id_t::handshake);

        auto process_message_queue = [&]
        {

            while(!message_queue.empty())
            {
                auto message = std::move(message_queue.front());
                if (message.second.type() == ssl_data_type_t::encrypted)
                {
                    switch(message.first)
                    {
                        case ssl_role_t::client:
                            client_connection->send_message(message.second);
                        break;
                        case ssl_role_t::server:
                            server_connection->send_message(message.second);
                        break;
                    }
                }
                else
                {
                    std::string str(static_cast<const char*>(message.second.data()), message.second.size());
                    std::cout << "message: " << str << std::endl;
                }
                message_queue.pop();
            }
        };

        process_message_queue();

        std::string test_string = "Vasiliy Kurbatov";
        const_ssl_message test_message(test_string.data()
                                       , test_string.size()
                                       , ssl_data_type_t::application);

        fingerprint_t server_fingerpring;
        fingerprint_t client_fingerpring;
        fingerprint_t server_peer_fingerpring;
        fingerprint_t client_peer_fingerpring;

        server_fingerpring = server_connection->get_fingerprint(fingerprint_direction_t::self
                                                                , hash_method_t::sha256);
        server_peer_fingerpring = server_connection->get_fingerprint(fingerprint_direction_t::peer
                                                                     , hash_method_t::sha256);
        client_fingerpring = client_connection->get_fingerprint(fingerprint_direction_t::self
                                                                , hash_method_t::sha256);
        client_peer_fingerpring = client_connection->get_fingerprint(fingerprint_direction_t::peer
                                                                    , hash_method_t::sha256);

        bool cmp1 = server_fingerpring == client_peer_fingerpring;
        bool cmp2 = client_fingerpring == server_peer_fingerpring;

        process_message_queue();

        client_connection->send_message(test_message);

        process_message_queue();

        server_connection->send_message(test_message);

        process_message_queue();

        client_connection->control(ssl_control_id_t::shutdown);

        process_message_queue();

        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    */
    return;

}

void test()
{
    test4();
}

}
