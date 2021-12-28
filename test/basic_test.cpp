#include "/home/pi/cpputest/include/CppUTest/TestHarness.h"
#include <string.h>
#include <stdbool.h>
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

TEST(basic_test_group, test_that_all_fpga_instructions_are_found)
{
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET",
        "EXIT", "HELP", "SELECT_SLAVE", "SHOW_SLAVE"};
    int i = 0;
    while (i < 37) {
        char *command = (char *)expected_commands[i];
        bool ret = instruction_exists(command);
        CHECK(ret);
        i++;
    }
}

TEST(basic_test_group, test_that_all_fpga_instructions_in_binary_are_found)
{
    char binary_data[10];
    memset(binary_data, '\0', sizeof(binary_data));
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET",
        "EXIT", "HELP", "SELECT_SLAVE", "SHOW_SLAVE"};
    const char *expected_binary[] = {
        "000111", "000101", "000001", "001001", "000011", "001011", "001010", "001111",
        "000100", "001000", "001101", "001100", "000010", "001110", "000110", "111110",
        "111001", "111000", "110000", "111101", "111010", "0100", "0101", "110001", "110010",
        "110011", "101000", "000000", "000000", "000000", "000000", "000000", "000000",
        "000000", "000000", "000000", "000000"};
    int i = 0;
    while (i < 37) {
        char *command = (char *)expected_commands[i];
        bool ret = get_command(command, binary_data);
        CHECK(ret);
        STRCMP_EQUAL(expected_binary[i], binary_data);
        i++;
    }
}

TEST(basic_test_group, test_that_expected_number_of_arguments_for_all_fpga_instructions_are_found)
{
    int num_args = 0;
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET",
        "EXIT", "HELP", "SELECT_SLAVE", "SHOW_SLAVE"};
    const int expected_args[] = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
        2, 2, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0};
    int i = 0;
    while (i < 37) {
        char *command = (char *)expected_commands[i];
        num_args = get_expected_num_of_arguments(command);
        CHECK_EQUAL(expected_args[i], num_args);
        i++;
    }
}
