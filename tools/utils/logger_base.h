#ifndef LOGGER_BASE_H
#define LOGGER_BASE_H

/*#include <string>*/
#include <iostream>
//#include <logger/logger_tools.h>

#define LOG(a)	std::cout << "[" << #a << "] "
#define LOG_END << std::endl;

#define LOG_T LOG(trace)
#define LOG_D LOG(debug)
#define LOG_I LOG(info)
#define LOG_W LOG(warning)
#define LOG_E LOG(error)
#define LOG_F LOG(fatal)

namespace pt::utils
{


}

#endif // LOGGER_BASE_H
