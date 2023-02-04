#include <string.h>
#include <stdint.h>
extern "C"
{
#include "process_command.h"
#include "common_data.h"
#include "defines.h"
}
#include "../cpputest/include/CppUTest/TestHarness.h"
#include "../cpputest/include/CppUTestExt/MockSupport.h"

TEST_GROUP(test_process_command_group)
{
    void teardown()
    {
        mock().clear();
    }
};

TEST(test_process_command_group, test_command_validation)
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

TEST(test_process_command_group, test_is_hw_command)
{
    const char *commands[] = {"ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
                              "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
                              "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
                              "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
                              "READ_STATUS", "READ_ADDRESS", "DUMP_MEM", "NOP",
                              "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET",
                              "DISABLE_RESET", "EXIT", "HELP", "SELECT_SLAVE",
                              "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"};
    const bool expected_return_values[] = {true, true, true, true, true, true, true, true, true,
                                           true, true, true, true, true, true, true, true, true,
                                           true, true, true, true, true, true, true, true, true,
                                           true, true, false, false, false, false, false, false,
                                           false, false, false, false};
    for (int idx = 0; idx < 39; idx++) {
        char *command = (char *)commands[idx];
        int expected_return_value = expected_return_values[idx];
        int ret = is_hw_command(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
}

TEST(test_process_command_group, test_is_sw_command)
{
    const char *commands[] = {"ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
                              "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
                              "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
                              "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
                              "READ_STATUS", "READ_ADDRESS", "DUMP_MEM", "NOP",
                              "READ_FILE", "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET",
                              "DISABLE_RESET", "EXIT", "HELP", "SELECT_SLAVE",
                              "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"};
    const bool expected_return_values[] = {false, false, false, false, false, false, false, false,
                                           false, false, false, false, false, false, false, false,
                                           false, false, false, false, false, false, false, false,
                                           false, false, false, false, false, true, true, true,
                                           true, true, true, true, true, true, true};
    for (int idx = 0; idx < 39; idx++) {
        char *command = (char *)commands[idx];
        int expected_return_value = expected_return_values[idx];
        int ret = is_sw_command(command);
        CHECK_EQUAL(ret, expected_return_value);
    }
}

TEST(test_process_command_group, test_process_sw_command)
{
    CHECK_EQUAL(process_sw_command("INVALID_COMMAND", NULL), 1);
    CHECK_EQUAL(process_sw_command("SELECT_SLAVE 0", NULL), 0);
    CHECK_EQUAL(process_sw_command("SELECT_SLAVE", NULL), 1);
    CHECK_EQUAL(process_sw_command("SELECT_SLAVE 2", NULL), 1);
    CHECK_EQUAL(process_sw_command("SHOW_SLAVE", NULL), 0);
    CHECK_EQUAL(process_sw_command("ENABLE_CLOCK", NULL), 0);
    CHECK_EQUAL(process_sw_command("DISABLE_CLOCK", NULL), 0);
    mock().expectOneCall("set_gpio_high").withParameter("pin", RESET_PIN).ignoreOtherParameters();
    CHECK_EQUAL(process_sw_command("ENABLE_RESET", NULL), 0);
    mock().expectOneCall("set_gpio_low").withParameter("pin", RESET_PIN).ignoreOtherParameters();
    CHECK_EQUAL(process_sw_command("DISABLE_RESET", NULL), 0);
    mock().expectOneCall("set_gpio_low").withParameter("pin", RESET_PIN).ignoreOtherParameters();
    CHECK_EQUAL(process_sw_command("EXIT", NULL), -1);
    CHECK_EQUAL(process_sw_command("HELP", NULL), 0);
    CHECK_EQUAL(process_sw_command("SET_CLK_FREQ", NULL), 1);
    CHECK_EQUAL(process_sw_command("SET_CLK_FREQ 5000", NULL), 0);
    CHECK_EQUAL(process_sw_command("SHOW_CLK_FREQ", NULL), 0);
    mock().checkExpectations();
}
