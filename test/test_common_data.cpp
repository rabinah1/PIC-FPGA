#include <string.h>
#include <stdint.h>
extern "C"
{
#include "common_data.h"
#include "defines.h"
}
#include "../cpputest/include/CppUTest/TestHarness.h"

TEST_GROUP(test_common_data_group)
{
};

TEST(test_common_data_group, test_get_num_instructions_slave_0)
{
    int num_instructions = get_num_instructions_slave_0();
    CHECK_EQUAL(num_instructions, 39);
}

TEST(test_common_data_group, test_get_num_instructions_slave_1)
{
    int num_instructions = get_num_instructions_slave_1();
    CHECK_EQUAL(num_instructions, 2);
}

TEST(test_common_data_group, test_get_slave_0_command_with_idx)
{
    char command[MAX_STRING_SIZE];
    char expected_command[MAX_STRING_SIZE];
    memset(command, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(expected_command, '\0', sizeof(char) * MAX_STRING_SIZE);
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE"};
    for (int idx = 0; idx < 29; idx++) {
        strcpy(command, get_slave_0_command(idx));
        strcpy(expected_command, expected_commands[idx]);
        int ret = strcmp(command, expected_command);
        CHECK_EQUAL(ret, 0);
    }
}

TEST(test_common_data_group, test_get_slave_1_command_with_idx)
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

TEST(test_common_data_group, test_that_expected_number_of_arguments_for_all_fpga_instructions_are_found)
{
    int num_args = 0;
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP", "READ_FILE"};
    const int expected_args[] = {
        2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 1, 1, 1, 1, 1, 1,
        2, 2, 0, 0, 1, 0, 0, 1};
    int idx = 0;
    while (idx < 29) {
        char *command = (char *)expected_commands[idx];
        num_args = get_expected_num_of_arguments(command);
        CHECK_EQUAL(expected_args[idx], num_args);
        idx++;
    }
}
