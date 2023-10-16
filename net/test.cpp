#include "test.h"
#include "net_engine_impl.h"
#include "socket/udp_transport_params.h"
#include "socket/i_udp_transport.h"

#include "utils/property_utils.h"
#include "utils/pointer_utils.h"
#include "utils/message_sink_impl.h"
#include "utils/enum_utils.h"
#include "utils/time_utils.h"
#include "utils/hash_utils.h"
#include "utils/task_manager_impl.h"
#include "utils/timer_manager_impl.h"
#include "utils/message_command_impl.h"

#include "net_message_types.h"
#include "core/i_message_source.h"
#include "core/i_message_event.h"
#include "core/event_channel_state.h"
#include "socket/socket_packet_impl.h"


#include "ice/ice_transport_factory.h"
#include "ice/ice_config.h"
#include "ice/ice_server_params.h"
#include "ice/ice_gathering_state_event.h"
#include "ice/ice_gathering_command.h"
#include "ice/ice_transport_params.h"

#include "utils/endian_utils.h"

#include "tools/io/net/net_utils.h"

#include <iostream>
#include <thread>
#include <cstring>
#include <set>
#include <list>

namespace mpl::net
{

void test1()
{

    auto& engine = net_engine_impl::get_instance();

    udp_transport_params_t socket_param_1;
    udp_transport_params_t socket_param_2;

    socket_param_1.local_endpoint.socket_address = socket_address_t::from_string("192.168.0.103:0");
    socket_param_1.options.reuse_address = true;
    socket_param_2.local_endpoint.socket_address = socket_address_t::from_string("localhost:0");
    socket_param_1.options.reuse_address = true;

    if (auto udp_factory = engine.create_factory(transport_id_t::udp
                                                 , nullptr))
    {
        engine.start();

        auto socket_property_1 = utils::property::serialize(socket_param_1);

        if (i_udp_transport::u_ptr_t socket_1 = utils::static_pointer_cast<i_udp_transport>(udp_factory->create_transport(*socket_property_1)))
        {
            if (socket_1->control(channel_control_t::open()))
            {
                socket_param_2.remote_endpoint = socket_1->local_endpoint();
                auto socket_property_2 = utils::property::serialize(socket_param_2);

                if (i_udp_transport::u_ptr_t socket_2 = utils::static_pointer_cast<i_udp_transport>(udp_factory->create_transport(*socket_property_2)))
                {
                    if (socket_2->control(channel_control_t::open()))
                    {
                        auto message_handler_1 = [&](const i_message& message)
                        {
                            switch(message.category())
                            {
                                case message_category_t::event:
                                {
                                    if (static_cast<const i_message_event&>(message).event().event_id == event_channel_state_t::id)
                                    {
                                        auto& state_event = static_cast<const event_channel_state_t&>(static_cast<const i_message_event&>(message).event());
                                        std::cout << "Socket #1: state: " << utils::enum_to_string(state_event.state) << ", reason: " << state_event.reason << std::endl;

                                        return true;
                                    }
                                }
                                break;
                                case message_category_t::packet:
                                {
                                    if (message.subclass() == message_net_class)
                                    {
                                        auto& net_packet = static_cast<const i_net_packet&>(message);
                                        if (net_packet.transport_id() == transport_id_t::udp)
                                        {
                                            auto& socket_packet = static_cast<const i_socket_packet&>(message);
                                            std::string_view message_string(static_cast<const char*>(socket_packet.data())
                                                                                                     , socket_packet.size());
                                            std::cout << "Socket #1: message: " << message_string << ", from: " << socket_packet.endpoint().socket_address.to_string() << std::endl;

                                            return true;
                                        }
                                    }
                                }
                                break;
                                default:;
                            }

                            return false;
                        };

                        auto message_handler_2 = [&](const i_message& message)
                        {
                            switch(message.category())
                            {
                                case message_category_t::event:
                                {
                                    if (static_cast<const i_message_event&>(message).event().event_id == event_channel_state_t::id)
                                    {
                                        auto& state_event = static_cast<const event_channel_state_t&>(static_cast<const i_message_event&>(message).event());
                                        std::cout << "Socket #2: state: " << utils::enum_to_string(state_event.state) << ", reason: " << state_event.reason << std::endl;

                                        return true;
                                    }
                                }
                                break;
                                case message_category_t::packet:
                                {
                                    if (message.subclass() == message_net_class)
                                    {
                                        auto& net_packet = static_cast<const i_net_packet&>(message);
                                        if (net_packet.transport_id() == transport_id_t::udp)
                                        {
                                            auto& socket_packet = static_cast<const i_socket_packet&>(message);
                                            std::string_view message_string(static_cast<const char*>(socket_packet.data())
                                                                                                     , socket_packet.size());
                                            std::cout << "Socket #2: message: " << message_string << ", from: " << socket_packet.endpoint().socket_address.to_string() << std::endl;

                                            socket_packet_impl loopback_socket_packet(smart_buffer(&socket_packet)
                                                                                      , socket_packet.endpoint());

                                            return socket_2->sink(0)->send_message(loopback_socket_packet);
                                        }
                                    }
                                }
                                break;
                                default:;
                            }

                            return false;
                        };

                        message_sink_impl socket_sink_1(message_handler_1);
                        message_sink_impl socket_sink_2(message_handler_2);

                        socket_1->source(0)->add_sink(&socket_sink_1);
                        socket_2->source(0)->add_sink(&socket_sink_2);

                        socket_1->control(channel_control_t::connect());
                        socket_2->control(channel_control_t::connect());

                        const std::string string_test = "Vasiliy Kurbatov message #";

                        for (auto i = 0; i < 1000; i++)
                        {
                            auto message = std::string(string_test).append(std::to_string(i));
                            socket_packet_impl packet(smart_buffer(message.data()
                                                                   , message.size())
                                                      , socket_endpoint_t(socket_type_t::udp
                                                                          , std::string("localhost:").append(std::to_string(socket_2->local_endpoint().socket_address.port))));

                            socket_1->sink(0)->send_message(packet);

                            std::this_thread::sleep_for(std::chrono::microseconds(100));
                        }

                        socket_2->control(channel_control_t::shutdown());
                        socket_1->control(channel_control_t::shutdown());

                        socket_2->control(channel_control_t::close());
                        socket_1->control(channel_control_t::close());

                        socket_1->source(0)->remove_sink(&socket_sink_1);
                        socket_2->source(0)->remove_sink(&socket_sink_2);
                    }
                }
            }
        }


        engine.stop();
    }
}

void test2()
{

    std::uint32_t value1 = 0x00345678;
    std::uint32_t value2 = utils::endian::big::get_value<std::uint32_t>(&value1, 3);
    std::uint32_t value3 = utils::endian::big::get_value<std::uint32_t>(&value2, 3);

    std::array<std::uint8_t, sizeof(std::uint32_t)> a1;
    std::array<std::uint8_t, sizeof(std::uint32_t)> a2;
    std::array<std::uint8_t, sizeof(std::uint32_t)> a3;

    std::memcpy(a1.data(), &value1, sizeof(value1));
    std::memcpy(a2.data(), &value2, sizeof(value2));
    std::memcpy(a3.data(), &value3, sizeof(value3));

    auto h1 = utils::calc_hash(a1.data(), a1.size());
    auto h2 = utils::calc_checksum(a1.data(), a1.size());
    auto h3 = utils::calc_crc32(a1.data(), a1.size());
    auto h4 = utils::calc_hmac_sha1(a1.data(), a1.size(), "Vasiliy");

    auto now = utils::time::get_ticks();
    auto address1 = io::utils::get_host_by_name("stun.l.google.com", ip_version_t::ip4);
    auto delay = utils::time::get_ticks() - now;

    io::ip_endpoint_t endpoint2(std::string("stun.l.google.com:19302"));

    std::cout << "address1: " << address1.to_string()
              << ", endpoint2: " << endpoint2.to_string()
              << ", delay: " << std::to_string(durations::to_microseconds(delay))
              << std::endl;

    return;
}

struct test_t
{
    std::int32_t a;
    std::int32_t b;

    test_t(std::int32_t a
           , std::int32_t b = 0)
        : a(a)
        , b(b)
    {
        std::cout << "test_t(" << a
                  << ", " << b
                  << "): " << this
                  << std::endl;
    }

    ~test_t()
    {
        std::cout << "~test_t(" << a
                  << ", " << b
                  << "): " << this
                  << std::endl;
    }

    bool operator == (const test_t& other) const
    {
        return a == other.a
                && b == other.b;
    }

    bool operator < (const test_t& other) const
    {
        return b < other.b;
    }

};

void test3()
{
    std::list<test_t> test_list;

    test_list.emplace_back(8, 8);
    test_list.emplace_back(1, 1);
    test_list.emplace_back(4, 4);
    test_list.emplace_back(2, 2);
    test_list.emplace_back(5, 5);

    std::cout << "before sort: " << std::endl;
    for (auto& t : test_list)
    {
        std::cout << "a: " << t.a
                  << ", b:" << t.b
                  << std::endl;
    }

    auto it = std::find_if(test_list.begin(), test_list.end(), [](auto&& args) { return args.a == 2; } );
    std::cout << "found item: " << "a: " << it->a << ", b: " << it->b << std::endl;

    test_list.sort();

/*
    std::sort(test_list.begin()
              , test_list.end()
              , [&](const test_t& l, const test_t& r)
                {
                    return l.a > r.a;
                });*/


    std::cout << "after sort: " << std::endl;
    for (auto& t : test_list)
    {
        std::cout << "a: " << t.a
                  << ", b:" << t.b
                  << std::endl;
    }

    while (it != test_list.end())
    {
        std::cout << "found item: " << "a: " << it->a << ", b: " << it->b << std::endl;
        it++;
    }
}

void test4()
{
    auto& engine = net_engine_impl::get_instance();

    engine.start();

    const ice_server_params_t::array_t ice_servers =
    {
        { "stun:stun1.l.google.com:19302" },
        { "stun:stun2.l.google.com:19302" },
        { "stun:stun3.l.google.com:19302" },
        { "stun:stun.l.google.com:19302" }
    };

    auto tasks = task_manager_factory::get_instance().create_manager({});
    auto timers = timer_manager_factory::get_instance().create_timer_manager({}
                                                                             , *tasks);


    timers->start();

    auto socket_factory = engine.create_factory(transport_id_t::udp
                                                , nullptr);

    ice_transport_factory ice_factory(ice_config_t(ice_servers)
                                      , *socket_factory
                                      , *timers);




    ice_transport_params_t ice_params_1;
    ice_transport_params_t ice_params_2;
    ice_params_1.local_endpoint.auth = ice_auth_params_t::generate();
    ice_params_2.local_endpoint.auth = ice_auth_params_t::generate();
    ice_params_1.remote_endpoint.auth = ice_params_2.local_endpoint.auth;
    ice_params_2.remote_endpoint.auth = ice_params_1.local_endpoint.auth;

    ice_params_1.sockets.emplace_back(socket_endpoint_t(socket_type_t::udp
                                                        , socket_address_t("0.0.0.0:0")));
    ice_params_2.sockets.emplace_back(socket_endpoint_t(socket_type_t::udp
                                                        , socket_address_t("0.0.0.0:0")));


    auto ice_property_1 = utils::property::serialize(ice_params_1);
    auto ice_property_2 = utils::property::serialize(ice_params_2);

    auto ice_connection_1 = ice_factory.create_transport(*ice_property_1);
    auto ice_connection_2 = ice_factory.create_transport(*ice_property_2);

    ice_connection_1->control(channel_control_t::open());

    utils::time::sleep(durations::seconds(1));

    utils::time::sleep(durations::seconds(10));

    engine.stop();

    return;


/*

    for (auto& s : sockets_2)
    {
        ice_params_2.component_id = s.first;
        if (auto ice_property_2 = wbs::utils::property::serialize(ice_params_2))
        {
            if (auto ice_connection = std::static_pointer_cast<i_ice_connection>(ice.query_connection(*ice_property_2)))
            {
                ice_connection->set_sockets(s.second);
                connections_2.emplace_back(std::move(ice_connection));
            }
        }
    }

    if (!connections_1.empty()
            && !connections_2.empty())
    {
        auto ice_connection_1 = connections_1.front();
        auto ice_connection_2 = connections_2.front();

        auto message_handler = [&](const std::string& name, const i_rtc_message& message)
        {
            switch(message.category())
            {
                case rtc_message_category_t::packet:
                {
                    const auto& packet = message.cast<i_rtc_message_packet>();
                    std::cout << name << ": packet bytes: " << packet.size() << std::endl;
                }
                break;
                case rtc_message_category_t::event:
                {
                    const auto& event = message.cast<i_rtc_message_event>();
                    switch(event.event().event_id)
                    {
                        case rtc_event_id_t::channel_state:
                            std::cout << name << ": channel state: " << enum_to_string(event.event().cast<rtc_event_channel_state_t>().state) << std::endl;
                        break;
                        case rtc_event_id_t::gathering_state:
                            std::cout << name << ": gathering state: " << enum_to_string(event.event().cast<rtc_event_gathering_state_t>().state) << std::endl;
                        break;
                    }
                }
            }

            return true;
        };

        auto message_handler_1 = [&](const i_rtc_message& message)
        {
            return message_handler("connection #1", message);
        };

        auto message_handler_2 = [&](const i_rtc_message& message)
        {
            return message_handler("connection #2", message);
        };

        custom_message_sender ice_sender_1(std::move(message_handler_1));
        custom_message_sender ice_sender_2(std::move(message_handler_2));

        ice_connection_1->add_sender(&ice_sender_1);
        ice_connection_2->add_sender(&ice_sender_2);

        ice_connection_1->control(channel_control_t::open());
        ice_connection_2->control(channel_control_t::open());

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        for (const auto& c : ice_connection_1->local_candidates())
        {
            ice_connection_2->add_remote_candidate(c);
        }

        for (const auto& c : ice_connection_2->local_candidates())
        {
            ice_connection_1->add_remote_candidate(c);
        }

        auto ice_property_1 = wbs::utils::property::create_property(property_type_t::object);
        auto ice_property_2 = wbs::utils::property::create_property(property_type_t::object);

        ice_connection_1->get_params(*ice_property_1);
        ice_connection_2->get_params(*ice_property_2);

        property_merger(*ice_property_1
                        , *ice_property_2).merge("remote.auth"
                                                 , "local.auth");

        property_merger(*ice_property_2
                        , *ice_property_1).merge("remote.auth"
                                                 , "local.auth");

        ice_connection_1->set_params(*ice_property_1);
        ice_connection_2->set_params(*ice_property_2);

        ice_connection_1->control(channel_control_t::connect());

        std::this_thread::sleep_for(std::chrono::milliseconds(500));

        ice_connection_2->control(channel_control_t::connect());

        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        ice_connection_2->control(channel_control_t::shutdown());

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        ice_connection_2->control(channel_control_t::connect());

        std::this_thread::sleep_for(std::chrono::milliseconds(2000));

        auto count = 1000;

        while(count-- > 0)
        {
            auto& connection = count % 2 == 0
                    ? *ice_connection_1
                    : *ice_connection_2;

            if (connection.state() == channel_state_t::connected)
            {
                std::string test_data = "Test data #";
                test_data.append(std::to_string(1000 - count));
                smart_buffer buffer(test_data.data()
                                    , test_data.size());
                const_raw_packet packet(buffer);
                connection.send_message(packet);

            }

            std::this_thread::sleep_for(std::chrono::milliseconds(250));
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(60000));

        ice_connection_2->control(channel_control_t::close());
        ice_connection_1->control(channel_control_t::close());
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(5000));*/

}

void test()
{

test4();

}

}
