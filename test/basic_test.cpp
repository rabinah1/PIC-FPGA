#include <string.h>
#include <stdint.h>
extern "C"
{
#include "functions.h"
#include "defines.h"
}
#include "C:/Users/henry/PIC-FPGA/cpputest/include/CppUTest/TestHarness.h"

TEST_GROUP(basic_test_group)
{
};

TEST(basic_test_group, test_get_num_instructions_slave_0)
{
    int num_instructions = get_num_instructions_slave_0();
    CHECK_EQUAL(num_instructions, 37);
}

TEST(basic_test_group, test_get_num_instructions_slave_1)
{
    int num_instructions = get_num_instructions_slave_1();
    CHECK_EQUAL(num_instructions, 2);
}

TEST(basic_test_group, test_get_slave_0_command_with_idx)
{
    char command[MAX_STRING_SIZE];
    char expected_command[MAX_STRING_SIZE];
    memset(command, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(expected_command, '\0', sizeof(char) * MAX_STRING_SIZE);
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET",
        "EXIT", "HELP", "SELECT_SLAVE", "SHOW_SLAVE"};
    for (int idx = 0; idx < 37; idx++) {
        strcpy(command, get_slave_0_command(idx));
        strcpy(expected_command, expected_commands[idx]);
        int ret = strcmp(command, expected_command);
        CHECK_EQUAL(ret, 0);
    }
}

TEST(basic_test_group, test_get_slave_1_command_with_idx)
{
    char command[MAX_STRING_SIZE];
    char expected_command[MAX_STRING_SIZE];
    memset(command, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(expected_command, '\0', sizeof(char) * MAX_STRING_SIZE);
    const char *expected_commands[] = {"read_temperature", "echo"};
    for (int idx = 0; idx < 2; idx++) {
        strcpy(command, get_slave_1_command(idx));
        strcpy(expected_command, expected_commands[idx]);
        int ret = strcmp(command, expected_command);
        CHECK_EQUAL(ret, 0);
    }
}

TEST(basic_test_group, test_convert_binary_to_decimal_nonzero)
{
    volatile int binary_data[8] = {1, 1, 0, 0, 0, 0, 1, 1};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 195);
}

TEST(basic_test_group, test_convert_binary_to_decimal_zero)
{
    volatile int binary_data[8] = {0};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 0);
}

TEST(basic_test_group, test_convert_decimal_to_binary_8_bits)
{
    uint32_t decimal_data = 123;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "01111011";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(basic_test_group, test_convert_decimal_to_binary_8_bits_zero)
{
    uint32_t decimal_data = 0;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "00000000";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(basic_test_group, test_convert_decimal_to_binary_16_bits)
{
    uint32_t decimal_data = 1110;
    int num_bits = 16;
    char binary_data[18] = {0};
    char correct_data[18] = "0000010001010110";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
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
    int idx = 0;
    while (idx < 37) {
        char *command = (char *)expected_commands[idx];
        bool ret = get_command_in_binary(command, binary_data);
        CHECK(ret);
        STRCMP_EQUAL(expected_binary[idx], binary_data);
        idx++;
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
    int idx = 0;
    while (idx < 37) {
        char *command = (char *)expected_commands[idx];
        num_args = get_expected_num_of_arguments(command);
        CHECK_EQUAL(expected_args[idx], num_args);
        idx++;
    }
}

TEST(basic_test_group, test_binary_command_creation)
{
    const char *commands[] = {"ADDWF 1 5", "COMF 0 50", "ADDLW 123",
                              "IORLW 27", "BCF 0 12", "BSF 3 99", "READ_WREG"};
    const char *expected_binary_strings[] = {"00011110000101", "00100100110010", "11111001111011",
                                             "11100000011011", "01000000001100", "01010111100011",
                                             "11000100000000"};
    char binary_command[BINARY_COMMAND_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    for (int idx = 0; idx < 7; idx++) {
        char *command = (char *)commands[idx];
        char *expected_binary = (char *)expected_binary_strings[idx];
        bool ret = create_binary_command(command, binary_command, instruction);
        CHECK_TRUE(ret);
        STRCMP_EQUAL(expected_binary, binary_command);
    }
}

TEST(basic_test_group, test_slave_commands_processing)
{
    const char *commands[] = {"NOT_SLAVE_COMMAND", "SELECT_SLAVE 0", "SELECT_SLAVE 1",
                              "SELECT_SLAVE 2", "SHOW_SLAVE"};
    const int expected_return_values[] = {-1, 0, 0, 1, 0};
    for (int idx = 0; idx < 5; idx++) {
        char *command = (char *)commands[idx];
        int expected_return_value = expected_return_values[idx];
        int ret = handle_slave_commands(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
}

TEST(basic_test_group, test_command_validation)
{
    const char *slave_0_test_commands[] = {"MOVLW 123", "MOVLW INVALID", "ADDWF 1 10", "ADDWF A 14",
                                           "READ_WREG", "INVALID_ARGS 1 2 3 4 5", "ADDLW 1 3"};
    const char *slave_1_test_commands[] = {"read_temperature", "echo test", "invalid"};
    const char *invalid_slave_test_commands[] = {"command_a", "command_b", "command_c"};
    const bool expected_return_values_slave_0[] = {true, false, true, false, true, false, false};
    const bool expected_return_values_slave_1[] = {true, true, true};
    const bool expected_return_values_invalid_slave[] = {false, false, false};
    set_slave_id(SLAVE_ID_FPGA);
    for (int idx = 0; idx < 7; idx++) {
        char *command = (char *)slave_0_test_commands[idx];
        bool expected_return_value = expected_return_values_slave_0[idx];
        bool ret = is_command_valid(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
    set_slave_id(SLAVE_ID_ARDUINO);
    for (int idx = 0; idx < 3; idx++) {
        char *command = (char *)slave_1_test_commands[idx];
        bool expected_return_value = expected_return_values_slave_1[idx];
        bool ret = is_command_valid(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
    set_slave_id(2);
    for (int idx = 0; idx < 3; idx++) {
        char *command = (char *)invalid_slave_test_commands[idx];
        bool expected_return_value = expected_return_values_invalid_slave[idx];
        bool ret = is_command_valid(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
}
