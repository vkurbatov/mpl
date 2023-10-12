#include "visca_test.h"

#include "visca_device.h"


#include "tools/serial/serial_device.h"
#include "tools/base/string_base.h"

//#include <chrono>
#include <thread>
#include <iostream>
#include <cstdio>

namespace visca
{

static std::string hex_dump(const std::vector<std::uint8_t>& dump)
{
    return portable::hex_dump(dump.data(), dump.size());
}

void test1()
{

}

void test2()
{
    visca::visca_config_t cfg(1000
                              , 0x10
                              , 0x10);


    visca::visca_device device(cfg);

    if (device.open("/dev/ttyUSB0"))
    {
        std::uint16_t id = 0;

        std::int16_t pan = 0;
        std::int16_t tilt = 0;
        std::uint16_t zoom = 0;

        if (device.set_pan_tilt(500, 500))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            device.pan_tilt_stop();
        }

        if (device.set_zoom(10))
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            device.zoom_stop();
        }


        if (device.get_pan_tilt(pan, tilt))
        {
            std::uint16_t zoom = 0;
            device.get_zoom(zoom);

            std::cout << "pan = " << pan << ", tilt = " << tilt << ", zoom = " << zoom << std::endl;
        }
    }
}



}
