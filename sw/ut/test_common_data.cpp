#include <string.h>
#include <stdint.h>
extern "C"
{
#include "common_data.h"
#include "defines.h"
}
#include "cpputest/include/CppUTest/TestHarness.h"

TEST_GROUP(test_common_data_group)
{
};

TEST(test_common_data_group, test_get_num_instructions_slave_0)
{
    int num_instructions = get_num_instructions_slave_0();
    CHECK_EQUAL(num_instructions, 40);
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
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_RAM",
        "DUMP_EEPROM", "NOP", "READ_FILE"
    };

    for (int idx = 0; idx < 30; idx++) {
        strcpy(command, get_slave_0_command(idx));
        strcpy(expected_command, expected_commands[idx]);
        int ret = strcmp(command, expected_command);
        CHECK_EQUAL(ret, 0);
    }
}
