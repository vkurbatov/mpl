#include <iostream>
#include "test.h"
#include "convert_utils.h"
#include "property_value_impl.h"
#include "property_tree_impl.h"

#include "property_writer.h"

#include "convert_utils.h"
#include "audio_format_impl.h"
#include "tools/base/any_base.h"

#include <string>

namespace mpl
{

void test1()
{
    auto test_tree = property_helper::create_tree();
    property_writer writer(*test_tree);

    writer.set<std::string>("name.first", "Vasiliy");
    writer.set<std::string>("name.second", "Kurbatov");
    writer.set<std::int32_t>("name.age", 40);

    property_reader reader(*test_tree);

    std::cout << *reader.get<std::string>("name.first") << " "
              << *reader.get<std::string>("name.second") << " "
              << *reader.get<std::int32_t>("name.age") << " "
              << std::endl;

}

void test2()
{
    auto convert_test = [](auto& in, auto& out)
    {
        if (utils::convert(in, out))
        {
            std::cout << "forward conversion: from " << in << " to " << out << " completed" << std::endl;
            if (utils::convert(out, in))
            {
                std::cout << "backward conversion: from " << out << " to " << in << " completed" << std::endl;
            }
        }
    };

    std::int8_t i8_value = 1;
    std::int16_t i16_value = 2;
    std::int32_t i32_value = 3;
    std::int64_t i64_value = 4;
    std::uint8_t u8_value = 5;
    std::uint16_t u16_value = 6;
    std::uint32_t u32_value = 7;
    std::uint64_t u64_value = 8;
    float r32_value = 9.0f;
    double r64_value = 10.0;
    long double r96_value = 11.0;
    std::string s_value = "12";
    std::vector<std::uint8_t> hex_test = { 0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef };

    std::string test_string;

    convert_test(i8_value, test_string);
    convert_test(i16_value, test_string);
    convert_test(i32_value, test_string);
    convert_test(i64_value, test_string);
    convert_test(u8_value, test_string);
    convert_test(u16_value, test_string);
    convert_test(u32_value, test_string);
    convert_test(u64_value, test_string);
    convert_test(r32_value, test_string);
    convert_test(r64_value, test_string);
    convert_test(r96_value, test_string);
    convert_test(s_value, test_string);

    test_string.clear();

    if (utils::convert(hex_test, test_string))
    {
        std::cout << "forward conversion hex " << test_string << " completed" << std::endl;
        if (utils::convert(test_string, hex_test))
        {
            std::cout << "backward conversion hex " << test_string << " completed" << std::endl;
        }
    }



    return;
}

void test3()
{
    base::any any1, any2, any3;

    any1 = 1;
    base::any any11 = any1;
    any2 = 4.567;
    base::any any22(any2);
    any3 = std::string("vasiliy");
    auto any33(any3);


    auto cmp1 = any1 == any2;
    auto cmp2 = any2 == any3;
    auto cmp3 = any3 == any1;

    auto cmp4 = any1 == any11;
    auto cmp5 = any2 == any22;
    auto cmp6 = any3 == any33;

    auto v1 = any1.cast<std::int32_t>();
    auto v2 = any2.cast<double>();
    auto v3 = any3.cast<std::string>();

    auto pv1 = any1.cast<std::int32_t*>();
    auto pv2 = any2.cast<double*>();
    auto pv3 = any3.cast<std::string*>();

    auto r1 = std::is_pointer<std::int32_t>::value;
    auto r2 = std::is_pointer<std::int32_t*>::value;

    return;

    /*
    audio_format_impl audio_format(audio_format_id_t::opus, 48000, 2);
    auto clone_format = audio_format.clone();
    auto& a_clone = static_cast<i_audio_format&>(*clone_format);
    auto& a_clone2 = static_cast<i_media_format&>(audio_format);

    auto cpb = audio_format.is_compatible(*clone_format);

    auto eq = audio_format.is_equal(*clone_format);

    return;
*/
}

void test4()
{

    option_impl option1;
    option_impl option2;

    option1.set(1, 123);
    option1.set(2, 456.7);
    option1.set(3, true);
    option1.set(4, std::string("Vasiliy"));

    option2.set(5, 321);
    option2.set(6, 7.654);
    option2.set(7, false);
    option2.set(8, std::string("Kurbatov"));

    auto cmp1 = option1.is_equal(option2);
    auto c1 = option1.merge(option2);
    auto c2 = option2.merge(option1);
    auto cmp2 = option1.is_equal(option2);


    return;
}

void test()
{
    //test1();
    test4();
}

}
