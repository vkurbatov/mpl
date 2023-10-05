#include "test.h"

#include "time_utils.h"

#include "fifo_reader_impl.h"
#include "fifo_writer_impl.h"
#include "shared_data_impl.h"

#include "smart_buffer.h"

#include "packetizer.h"
#include "depacketizer.h"

#include <vector>
#include <thread>
#include <iostream>

#include "ipc_manager_impl.h"

#include "sq/sq_packet_builder.h"
#include "sq/sq_parser.h"
#include "sq/sq_stitcher.h"

#include "common_utils.h"

#include "task_manager_impl.h"
#include "timer_manager_impl.h"

#include "tools/ipc/test.h"
#include "tools/base/random_base.h"

#include "tools/io/net/udp_link.h"
#include "tools/io/net/udp_link_config.h"
#include "tools/io/net/ip_endpoint.h"
#include "tools/io/io_core.h"


namespace mpl
{

namespace
{

void test1()
{
    std::cout << "Hi form core library" << std::endl;
}

void test2()
{
    std::vector<std::uint8_t>   array(100);


    shared_data_ref_impl    shared_data(array.data()
                                        , array.size());


    auto write_proc = [&shared_data]()
    {
        fifo_writer_impl writer(shared_data);
        auto counter = 1000;

        while(counter-- > 0)
        {
            std::string test_string = "Vasiliy Kurbatov ";
            test_string.append(std::to_string(counter));
            if (writer.push_data(test_string.data()
                                 , test_string.size()))
            {
                std::cout << "Push data: " << test_string << " completed!" << std::endl;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    };

    auto reader_proc = [&shared_data]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        fifo_reader_impl reader(shared_data);
        auto counter = 1000;

        while(counter-- > 0)
        {
            auto pending_size = reader.pending_size();
            switch(pending_size)
            {
                case 0:
                    std::cout << "Reader: no data" << std::endl;
                break;
                case i_fifo_buffer::overload:
                    std::cout << "Reader: overload data" << std::endl;
                    reader.reset();
                break;
                default:
                {
                    std::string test_string;
                    test_string.resize(20);
                    auto readed_size = reader.pop_data(test_string.data(), test_string.size());
                    if (readed_size == i_fifo_buffer::overload)
                    {
                        std::cout << "Reader: queue overload" << std::endl;
                        reader.reset();
                    }
                    else if (readed_size > 0)
                    {
                        std::cout << "Pop data: " << test_string << " completed!" << std::endl;
                    }
                    else
                    {
                        std::cout << "Reader: can't read from queue" << std::endl;
                    }
                }
            }


            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    };

    std::thread write_thread(write_proc);
    std::thread read_thread(reader_proc);

    write_thread.join();
    read_thread.join();

}

void test3()
{
    auto write_proc = []()
    {
        if (auto manager = ipc_manager_factory::get_instance().create_shared_data_manager("mpl", 100000000))
        {
            if (auto shared_data = manager->query_data("ipc_data_1", 1000000))
            {
                fifo_writer_impl writer(*shared_data);
                auto counter = 1000;

                while(counter-- > 0)
                {
                    std::string test_string = "Vasiliy Kurbatov ";
                    test_string.append(std::to_string(counter));
                    if (writer.push_data(test_string.data()
                                         , test_string.size()))
                    {
                        std::cout << "Push data: " << test_string << " completed!" << std::endl;
                    }

                    shared_data->notify();

                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    };

    auto reader_proc = []()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        if (auto manager = ipc_manager_factory::get_instance().create_shared_data_manager("mpl", 0))
        {
            if (auto shared_data = manager->query_data("ipc_data_1", 0))
            {
                fifo_reader_impl reader(*shared_data);
                auto counter = 1000;

                while(counter-- > 0)
                {
                    if (shared_data->wait(durations::milliseconds(50)))
                    {
                        auto pending_size = reader.pending_size();
                        switch(pending_size)
                        {
                            case 0:
                                std::cout << "Reader: no data" << std::endl;
                            break;
                            case i_fifo_buffer::overload:
                                std::cout << "Reader: overload data" << std::endl;
                                reader.reset();
                            break;
                            default:
                            {
                                std::string test_string;
                                test_string.resize(20);
                                auto readed_size = reader.pop_data(test_string.data(), test_string.size());
                                if (readed_size == i_fifo_buffer::overload)
                                {
                                    std::cout << "Reader: queue overload" << std::endl;
                                    reader.reset();
                                }
                                else if (readed_size > 0)
                                {
                                    std::cout << "Pop data: " << test_string << " completed!" << std::endl;
                                }
                                else
                                {
                                    std::cout << "Reader: can't read from queue" << std::endl;
                                }
                            }
                        }
                    }
                    else
                    {
                        std::cout << "Reader: wait failed" << std::endl;
                    }
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            }
        }
    };

    std::thread write_thread(write_proc);
    std::thread read_thread(reader_proc);

    write_thread.join();
    read_thread.join();

}

void test4()
{
    smart_buffer packet;

    packetizer packer(packet);

    std::int32_t n_value = 1;
    double db_value = 2.0;
    std::string s_value = "Vasiliy";
    std::vector<std::uint16_t> a_value = { 1, 2, 3, 4, 5};


    packer.add_value(n_value);
    packer.add_value(db_value);
    packer.add_value(s_value);
    packer.add_value(a_value);

    std::int32_t o_n_value = 0;
    double o_db_value = 0.0f;
    std::string o_s_value = {};
    std::vector<std::uint16_t> o_a_value = {};

    depacketizer depacker(packet);
    depacker.fetch_value(o_n_value);
    depacker.fetch_value(o_db_value);
    depacker.fetch_value(o_s_value);
    depacker.fetch_value(o_a_value);

    bool cmp1 = n_value == o_n_value;
    bool cmp2 = db_value == o_db_value;
    bool cmp3 = s_value == o_s_value;
    bool cmp4 = a_value == o_a_value;

    return;

}

void test5()
{
    constexpr std::size_t test_count = 200;
    std::vector<std::uint8_t> test_array;

    for (std::size_t n = 0; n < test_count; n++)
    {
        test_array.push_back(static_cast<std::uint8_t>(n));
    }

    sq::sq_packet_builder_t builder(0
                                    , 0
                                    , 50);

    auto packets = builder.build_fragments(test_array.data()
                                           , test_array.size());

    smart_buffer stream_buffer;

    for (const auto& p : packets)
    {
        stream_buffer.append_data(p.data()
                                  , p.size());
    }

    auto packet_handler = [&](sq::sq_packet&& packet)
    {
        if (packet.is_valid())
        {
            std::vector<std::uint8_t> data(static_cast<const std::uint8_t*>(packet.payload_data())
                                           , static_cast<const std::uint8_t*>(packet.payload_data()) + packet.payload_size());

            std::cout << "Packet #" << packet.id() << ": size: " << data.size() << std::endl;
        }
    };

    sq::sq_parser parser(packet_handler);

    for (std::size_t i = 0; i < stream_buffer.size() - 3; i += 4)
    {
        parser.push_stream(&stream_buffer[i]
                           , 4);
    }

    return;

}

std::uint32_t calc_cs(const void* data, std::size_t size)
{
    std::uint32_t cs = 0;

    while(size-- > 0)
    {
        cs += static_cast<const std::uint8_t*>(data)[size];
    }

    return cs;
}

void test6()
{


    auto frame_handler = [&](smart_buffer&& frame)
    {
        auto cs = calc_cs(frame.data()
                            , frame.size());

        std::cout << "on_frame: size: " << frame.size()
                  << ", cs: " << cs << std::endl;
    };

    sq::sq_stitcher stitcher(frame_handler);

    auto packet_handler = [&](sq::sq_packet&& packet)
    {
        if (packet.is_valid())
        {
            std::cout << "Packet #" << packet.id() << ": size: " << packet.payload_size() << std::endl;

            stitcher.push_packet(std::move(packet));
        }
    };

    sq::sq_parser parser(packet_handler);

    auto test_count = 1000;


    sq::sq_packet_builder_t builder(0
                                    , 0
                                    , 50);

    builder.packet_id = 65501;

    while (test_count-- > 0)
    {
        auto frame_size = 1 + base::utils::random<std::size_t>() % 199;
        std::vector<std::uint8_t> test_array(frame_size);
        std::uint32_t cs = 0;

        for (auto& v : test_array)
        {
            v = base::utils::random<std::uint8_t>();
            cs += v;
        }

        auto packets = builder.build_fragments(test_array.data()
                                               , test_array.size());

        std::cout << "frame create: size: " << test_array.size()
                  << ", cs: " << cs
                  << ", packets: " << packets.size() << std::endl;

        for (auto& p : packets)
        {
            parser.push_stream(p.data()
                               , p.size());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }


    return;
}

void test7()
{
    if (auto task_manager = task_manager_factory::get_instance().create_manager({}))
    {
        if (auto timers = timer_manager_factory::get_instance().create_timer_manager({}
                                                                                     , *task_manager))
        {
            if (timers->start())
            {

                auto timeout_1 = durations::milliseconds(1000);
                auto timeout_2 = durations::milliseconds(500);

                i_timer::u_ptr_t timer1 = nullptr;
                i_timer::u_ptr_t timer2 = nullptr;

                auto timer_handler_1 = [&]
                {
                    std::clog << "Timer 1 handle" << std::endl;
                    if (timer1 != nullptr)
                    {
                        timer1->start(timeout_1);
                    }
                };

                auto timer_handler_2 = [&]
                {
                    std::clog << "Timer 2 handle" << std::endl;
                    timer2->start(timeout_2);
                };

                timer1 = timers->create_timer(timer_handler_1);
                timer2 = timers->create_timer(timer_handler_2);

                timer1->start(0);
                std::this_thread::sleep_for(std::chrono::nanoseconds(1));
                timer1.reset();
                timer2->start(0);

                utils::time::sleep(durations::minutes(1));

                timers->stop();
            }
        }
    }
}

void test8()
{
    auto& core = io::io_core::get_instance();
    core.run();

    io::udp_link link1(core
                       , {});
    io::udp_link link2(core
                       , {});

    io::ip_endpoint_t ep1("127.0.0.1:0");
    io::ip_endpoint_t ep2("127.0.0.1:0");

    auto link_state_handler_1 = [](io::link_state_t state
                                   , const std::string_view& reason)
    {
        std::clog << "Link #1 state: " << static_cast<std::int32_t>(state) << std::endl;
    };

    auto link_state_handler_2 = [](io::link_state_t state
                                   , const std::string_view& reason)
    {
        std::clog << "Link #2 state: " << static_cast<std::int32_t>(state) << std::endl;
    };

    auto link_message_handler_1 = [&](const io::message_t& message
                                      , const io::endpoint_t& endpoint)
    {
        std::string_view msg(static_cast<const char*>(message.data())
                             , message.size());

        std::clog << "Link #1 message: " << msg
                  << " from: " << endpoint.to_string()
                  << std::endl;
    };

    auto link_message_handler_2 = [&](const io::message_t& message
                                      , const io::endpoint_t& endpoint)
    {
        std::string_view msg(static_cast<const char*>(message.data())
                             , message.size());

        std::clog << "Link #2 message: " << msg
                  << " from: " << endpoint.to_string()
                  << std::endl;

        link1.send_to(message
                      , endpoint);
    };

    link1.set_endpoint(ep1);
    link2.set_endpoint(ep2);

    link1.set_state_handler(link_state_handler_1);
    link2.set_state_handler(link_state_handler_2);

    link1.set_message_handler(link_message_handler_1);
    link2.set_message_handler(link_message_handler_2);

    const auto count = 1000;
    const std::string_view test_string = "Test string";

    if (link1.control(io::link_control_id_t::open)
            && link2.control(io::link_control_id_t::open))
    {
        if (link1.control(io::link_control_id_t::start)
                && link2.control(io::link_control_id_t::start))
        {
            for (auto i = 0; i < count; i++)
            {
                auto send_sting = std::string(test_string)
                        .append(" #")
                        .append(std::to_string(i));

                io::message_t msg(send_sting.data(), send_sting.size());

                link1.send_to(msg
                              , link2.local_endpoint());

                // std::this_thread::yield();
                std::this_thread::sleep_for(std::chrono::milliseconds(2));
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            link2.control(io::link_control_id_t::stop);
            link1.control(io::link_control_id_t::stop);
        }

        link2.control(io::link_control_id_t::close);
        link1.control(io::link_control_id_t::close);
    }


    core.stop();
}

void ipc_test()
{
    ipc::test();
}

}

void utils_test()
{
    test8();
}


}
