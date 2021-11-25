#include "/home/pi/cpputest/include/CppUTest/TestHarness.h"
#include <string.h>
extern "C"
{
#include "code.h"
}

TEST_GROUP(basic_test_group)
{
};

TEST(basic_test_group, test_convert_binary_to_decimal_nonzero)
{
    volatile int binary_data[8] = {1, 1, 0, 0, 0, 0, 1, 1};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 195);
}

TEST(basic_test_group, test_convert_binary_to_decimal_zero)
{
    volatile int binary_data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 0);
}

TEST(basic_test_group, test_convert_decimal_to_binary_8_bits)
{
    int decimal_data = 123;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "01111011";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(basic_test_group, test_convert_decimal_to_binary_8_bits_zero)
{
    int decimal_data = 0;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "00000000";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(basic_test_group, test_convert_decimal_to_binary_16_bits)
{
    int decimal_data = 1110;
    int num_bits = 16;
    char binary_data[18] = {0};
    char correct_data[18] = "0000010001010110";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}
