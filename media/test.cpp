#include <iostream>
#include "test.h"
#include "convert.h"
#include "property_value_impl.h"
#include "property_tree_impl.h"

namespace mpl
{

void test1()
{
    property_tree tree;

    const std::string key = "vasiliy.kurbatov";

    tree.set(key, property_value<std::int32_t>::create(12345));

    if (auto property = tree.get(key))
    {
        std::string string_value;
        if (convert<i_property, std::string>(*property.get(), string_value))
        {
            std::cout << "Value of [" << key << "]: " << string_value << std::endl;
        }
    }

}

void test()
{
    test1();
}

}
