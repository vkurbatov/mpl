#include "test.h"

#include <iostream>

namespace mpl
{

namespace 
{

void test1()
{
    std::cout << "Hi form core library" << std::endl;
}

}

void core_test()
{
    test1();
}

}
