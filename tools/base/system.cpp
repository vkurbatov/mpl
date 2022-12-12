#include "system.h"

#include <sys/times.h>
//#include <sys/vtimes.h>
#include <unistd.h>
#include <dirent.h>

#include <cstdint>
#include <fstream>

namespace base
{

namespace
{

struct cpu_info_t
{
    std::size_t num_processors;
    clock_t     cpu_clock;
    clock_t     sys_clock;
    clock_t     user_clock;

    cpu_info_t()
        : num_processors(0)
        , cpu_clock(0)
        , sys_clock(0)
        , user_clock(0)
    {
        reset_info();
    }

    bool reset_info()
    {
        std::ifstream cpu_info_file("/proc/cpuinfo", std::ios_base::in);
        num_processors = 0;

        if (cpu_info_file.is_open())
        {
            struct tms time_sample = {};
            cpu_clock = times(&time_sample);
            sys_clock = time_sample.tms_stime;
            user_clock = time_sample.tms_utime;

            std::string line;

            while(std::getline(cpu_info_file, line))
            {
                if (line.find("processor") == 0)
                {
                    num_processors++;
                }
            }

            cpu_info_file.close();
        }

        return num_processors > 0;
    }

    double get_cpu_usage()
    {
        struct tms time_sample = {};
        double percent = -1.0;

        clock_t now = times(&time_sample);

        if (num_processors > 0
                && now > cpu_clock
                && time_sample.tms_stime >= sys_clock
                && time_sample.tms_utime >= user_clock)
        {
            percent = (time_sample.tms_stime - sys_clock) +
                (time_sample.tms_utime - user_clock);
            percent /= (now - cpu_clock);
            percent /= num_processors;
            percent *= 100.0;

            cpu_clock = now;
            sys_clock = time_sample.tms_stime;
            user_clock = time_sample.tms_utime;
        }

        return percent;
    }
};

}

static cpu_info_t cpu_info;

double cpu_usage()
{
    return cpu_info.get_cpu_usage();
}

int64_t pid()
{
    return getpid();
}

std::size_t get_open_handles(std::int64_t pid)
{
    std::size_t count = 0;
    std::string path = std::string("/proc/").append(std::to_string(pid)).append("/fd");

    if (auto dp = opendir(path.c_str()))
    {
         while (auto ep = readdir(dp))
         {
            if (ep->d_name[0] != 0)
            {
                count++;
            }
         }
         closedir(dp);
    }

    return count;
}


}
