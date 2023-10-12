#include "test.h"
#include "net_engine_impl.h"
#include "socket/udp_transport_params.h"
#include "socket/i_udp_transport.h"

#include "utils/property_utils.h"
#include "utils/pointer_utils.h"
#include "utils/message_sink_impl.h"
#include "utils/enum_utils.h"

#include "utils/time_utils.h"

#include "core/i_message_source.h"
#include "core/i_message_event.h"
#include "core/event_channel_state.h"
#include "socket/socket_packet_impl.h"

#include "utils/endian_utils.h"

#include "tools/io/net/net_utils.h"

#include <iostream>
#include <thread>
#include <cstring>

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

void test()
{

test2();

}

}
