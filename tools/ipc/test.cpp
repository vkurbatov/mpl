#include "test.h"
#include "ipc_base.h"
#include "ipc_manager.h"
#include "ipc_segment.h"

#include <thread>

#include <iostream>
#include <cstring>

namespace pt::ipc
{

void test()
{
    std::string manager_name = "mpl_shm";
    std::size_t manager_size = 10000000;

    std::string segment_name = "seg1";
    std::size_t segment_size = 10000;

    auto write_proc = [&]()
    {
        ipc_manager::config_t manager_config(manager_name
                                             , manager_size);

        ipc_manager manager(manager_config);

        if (manager.is_valid())
        {
            ipc_segment segment = manager.create_segment(segment_name
                                                         , segment_size);
            if (segment.is_valid())
            {
                {
                    std::size_t count = 1000;
                    while(count-- > 0)
                    {
                        std::string write_string = "Kurbatov Vasiliy ";
                        if (auto data = segment.map())
                        {
                            std::uint8_t* ptr = static_cast<std::uint8_t*>(data);
                            write_string.append(std::to_string(count));

                            *static_cast<std::uint16_t*>(data) = write_string.size();
                            std::memcpy(ptr + 2
                                        , write_string.data()
                                        , write_string.size());

                            segment.unmap();
                            segment.notify(true);
                        }
                        std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    }
                }
            }
        }
    };

    auto reader_proc = [&]()
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        ipc_manager::config_t manager_config(manager_name);

        ipc_manager manager(manager_config);

        if (manager.is_valid())
        {
            ipc_segment segment = manager.create_segment(segment_name
                                                         , 0);
            if (segment.is_valid())
            {
                {
                    while(true)
                    {
                        auto to = 110 * 1000 * 1000;
                        if (segment.wait(to))
                        {
                            if (auto data = segment.map())
                            {
                                std::uint16_t test_size = *static_cast<const std::uint8_t*>(data);
                                const std::uint8_t* ptr = static_cast<const std::uint8_t*>(data) + 2;

                                std::string read_string;
                                read_string.resize(test_size);

                                std::memcpy(read_string.data()
                                            , ptr
                                            , test_size);

                                std::cout << "Read test string: " << read_string << std::endl;
                                segment.unmap();

                            }
                        }
                        else
                        {
                            std::cout << "Wait timeout: " << std::endl;
                        }
                    }
                }
            }
        }
    };

    std::thread writer_thread(write_proc);
    std::thread reader_thread(reader_proc);

    writer_thread.join();
    reader_thread.join();

    return;
}

}
