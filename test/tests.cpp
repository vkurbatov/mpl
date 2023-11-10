#include "tests.h"

#include "core/i_message_event.h"
#include "core/i_message_command.h"
#include "core/event_channel_state.h"
#include "core/i_message_sink.h"
#include "core/i_message_source.h"

#include "app/app_config.h"
#include "app/app_engine_impl.h"

#include "utils/pointer_utils.h"
#include "utils/time_utils.h"
#include "utils/enum_utils.h"
#include "utils/message_sink_impl.h"
#include "utils/property_writer.h"

#include "net/net_module_types.h"
#include "net/socket/socket_endpoint.h"
#include "net/i_net_module.h"
#include "net/i_transport_collection.h"
#include "net/ice/ice_types.h"
#include "net/ice/ice_auth_params.h"
#include "net/ice/ice_transport_params.h"
#include "net/ice/ice_gathering_state_event.h"
#include "net/ice/ice_gathering_command.h"
#include "net/ice/i_ice_transport.h"

#include <iostream>

namespace mpl::test
{

void ice_test()
{
    const net::ice_server_params_t::array_t ice_servers =
    {
        { "stun:stun1.l.google.com:19302" },
        { "stun:stun2.l.google.com:19302" },
        { "stun:stun3.l.google.com:19302" },
        { "stun:stun.l.google1.com:19302" }
    };

    app::app_config_t app_config;
    app_config.core_config.worker_count = 0;
    app_config.net_config.ice_config.auto_gathering = true;
    app_config.net_config.ice_config.ice_servers = ice_servers;

    if (app::i_app_engine::u_ptr_t app = app::app_engine_factory::get_instance().create_engine(app_config))
    {
        if (app->start())
        {
            if (net::i_net_module* net = static_cast<net::i_net_module*>(app->get_module(net::net_module_id)))
            {
                if (auto ice_factory = net->transports().get_factory(net::transport_id_t::ice))
                {
                    if (auto socket_packet_builder = net->create_packet_builder(net::transport_id_t::udp))
                    {
                        auto ice_params_1 = utils::property::create_property(property_type_t::object);
                        if (ice_params_1)
                        {

                            property_writer writer(*ice_params_1);
                            writer.append("sockets", net::udp_endpoint_t(net::socket_address_t::from_string("0.0.0.0:0")));
                            writer.set("mode", net::ice_mode_t::regular);
                            writer.set<std::uint8_t>("component_id", 1);
                            writer.set("local_endpoint.auth", net::ice_auth_params_t::generate());

                        }

                        auto ice_params_2 = utils::property::create_property(property_type_t::object);
                        if (ice_params_2)
                        {
                            property_writer writer(*ice_params_2);
                            writer.append("sockets", net::udp_endpoint_t(net::socket_address_t::from_string("0.0.0.0:0")));
                            writer.set("mode", net::ice_mode_t::aggressive);
                            writer.set<std::uint8_t>("component_id", 1);
                            writer.set("local_endpoint.auth", net::ice_auth_params_t::generate());
                        }

                        auto message_handler = [&](const std::string_view& name, const i_message& message)
                        {
                            switch(message.category())
                            {
                                case message_category_t::event:
                                {
                                    auto& message_event = static_cast<const i_message_event&>(message).event();
                                    switch(message_event.event_id)
                                    {
                                        case event_channel_state_t::id:
                                        {
                                            auto &event_channel_state = static_cast<const event_channel_state_t&>(message_event);
                                            std::cout << name
                                                      << ": channel state: " <<  utils::enum_to_string(event_channel_state.state)
                                                      << ", reason: " << event_channel_state.reason
                                                      << std::endl;
                                        }
                                        break;
                                        case net::ice_gathering_state_event_t::id:
                                        {
                                            auto &event_gathering_state = static_cast<const net::ice_gathering_state_event_t&>(message_event);
                                            std::cout << name
                                                      << ": gathering state: " <<  utils::enum_to_string(event_gathering_state.state)
                                                      << ", reason: " << event_gathering_state.reason
                                                      << std::endl;
                                        }
                                        break;
                                        default:;
                                    }
                                }
                                break;
                                case message_category_t::packet:
                                {
                                    if (message.module_id() == net::net_module_id)
                                    {
                                        auto& net_packet = static_cast<const net::i_net_packet&>(message);
                                        std::string_view text_message(static_cast<const char*>(net_packet.data())
                                                                                                , net_packet.size());
                                        std::cout << name
                                                  << ": packet: transport: " << utils::enum_to_string(net_packet.transport_id())
                                                  << ", size: " << net_packet.size()
                                                  << ", message: " << text_message
                                                  << std::endl;
                                    }
                                }
                                break;
                                default:;
                            }

                            return true;
                        };

                        message_sink_impl sink_1([&](auto&& ...args) { return message_handler("ice1", args...); } );
                        message_sink_impl sink_2([&](auto&& ...args) { return message_handler("ice2", args...); } );


                        net::i_ice_transport::u_ptr_t ice_connection_1 = utils::static_pointer_cast<net::i_ice_transport>(ice_factory->create_transport(*ice_params_1));
                        net::i_ice_transport::u_ptr_t ice_connection_2 = utils::static_pointer_cast<net::i_ice_transport>(ice_factory->create_transport(*ice_params_2));


                        ice_connection_1->source(0)->add_sink(&sink_1);
                        ice_connection_2->source(0)->add_sink(&sink_2);

                        ice_connection_1->control(channel_control_t::open());
                        ice_connection_2->control(channel_control_t::open());

                        utils::time::sleep(durations::seconds(1));

                        std::clog << "ice1 candidates:" << std::endl;
                        for (const auto& c : ice_connection_1->local_endpoint().candidates)
                        {
                            std::clog << c.to_string() << std::endl;
                        }

                        std::clog << "ice2 candidates:" << std::endl;
                        for (const auto& c : ice_connection_2->local_endpoint().candidates)
                        {
                            std::clog << c.to_string() << std::endl;
                        }

                        ice_connection_1->control(channel_control_t::connect());

                        utils::time::sleep(durations::seconds(2));
                        ice_connection_2->control(channel_control_t::connect());

                        const std::string_view test_string = "Kurbatov Vailiy Vladimirovich #";

                        while (true)
                        {
                            ice_connection_1->set_remote_endpoint(ice_connection_2->local_endpoint());
                            ice_connection_2->set_remote_endpoint(ice_connection_1->local_endpoint());

                            while(ice_connection_2->state() != channel_state_t::connected)
                            {
                                utils::time::sleep(durations::seconds(1));
                            }

                            for (auto i = 0; i < 10; i++)
                            {
                                auto test_message = std::string(test_string).append(std::to_string(i));

                                socket_packet_builder->set_packet_data(test_message.data()
                                                                       , test_message.size());

                                if (auto test_packet = socket_packet_builder->build_packet())
                                {
                                    ice_connection_1->sink(0)->send_message(*test_packet);
                                }

                                utils::time::sleep(durations::milliseconds(10));
                            }

                            ice_connection_2->control(channel_control_t::shutdown());

                            while(ice_connection_1->state() == channel_state_t::connected)
                            {
                                utils::time::sleep(durations::seconds(1));
                            }

                            ice_connection_2->control(channel_control_t::connect());

                            std::swap(ice_connection_1, ice_connection_2);
                        }

                        utils::time::sleep(durations::seconds(100));

                        ice_connection_2->control(channel_control_t::close());
                        ice_connection_1->control(channel_control_t::close());
                    }


                }
            }

            app->stop();
        }
    }

    return;
}

void tests()
{
    ice_test();
}

}
