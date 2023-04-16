#include "test.h"

#include "fifo_reader_impl.h"
#include "fifo_writer_impl.h"
#include "shared_data_impl.h"

#include <vector>
#include <thread>
#include <iostream>

#include "tools/ipc/test.h"

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

void ipc_test()
{
    ipc::test();
}

}

void core_test()
{
    // test2();
    ipc_test();
}

}
