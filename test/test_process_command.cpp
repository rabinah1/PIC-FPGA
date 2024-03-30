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
    void teardown() {
        mock().clear();
    }
};

// TEST(test_process_command_group, test_verify_command_syntax)
// {
//     const char *slave_0_valid_commands[] = {
//         "ADDWF 1   14", "ANDWF 0 30", "CLR   1    12", "COMF 0 89", "DECF 0 33", "DECFSZ 1 89",
//         "INCF 0 44", "INCFSZ 0 40", "IORWF 0 45", "MOVF   1 5", "RLF 4 100", "RRF 1 55",
//         "SUBWF 1 67", "SWAPF 1 78", "XORWF 1 89", "ADDLW 5", "ANDLW 78", "IORLW 99", "MOVLW   123",
//         "SUBLW 87", "XORLW 75", "BCF 4 78", "BSF 3   99", "READ_WREG", "READ_STATUS",
//         "READ_ADDRESS   78", "DUMP_RAM", "DUMP_EEPROM", "NOP", "READ_FILE    test_file.txt",
//         "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET", "EXIT", "HELP",
//         "SELECT_SLAVE    1", "SHOW_SLAVE", "SET_CLK_FREQ   4500", "SHOW_CLK_FREQ"
//     };
//     set_slave_id(SLAVE_ID_FPGA);
//     struct command_and_args cmd;

//     for (int idx = 0; idx < 40; idx++) {
//         memset(&cmd, 0, sizeof(cmd));
//         char *command = (char *)slave_0_valid_commands[idx];
//         bool ret = verify_command_syntax(command, &cmd);
//         CHECK_EQUAL(ret, true);
//     }
// }

// TEST(test_process_command_group, test_is_hw_command)
// {
//     const char *commands[] = {"ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
//                               "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
//                               "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
//                               "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
//                               "READ_STATUS", "READ_ADDRESS", "DUMP_RAM", "NOP",
//                               "READ_FILE", "read_temperature", "echo", "ENABLE_CLOCK",
//                               "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET", "EXIT", "HELP",
//                               "SELECT_SLAVE", "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"
//                              };
//     const bool expected_return_values[] = {true, true, true, true, true, true, true, true, true,
//                                            true, true, true, true, true, true, true, true, true,
//                                            true, true, true, true, true, true, true, true, true,
//                                            true, true, true, true, false, false, false, false,
//                                            false, false, false, false, false, false
//                                           };
//     struct command_and_args cmd;

//     for (int idx = 0; idx < 41; idx++) {
//         memset(&cmd, 0, sizeof(cmd));
//         char *command = (char *)commands[idx];
//         strcpy(cmd.command_name, command);
//         int expected_return_value = expected_return_values[idx];
//         int ret = is_expected_command_type(&cmd, "hw");
//         CHECK_EQUAL(ret, expected_return_value);
//     }
// }

// TEST(test_process_command_group, test_is_sw_command)
// {
//     const char *commands[] = {"ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
//                               "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
//                               "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
//                               "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
//                               "READ_STATUS", "READ_ADDRESS", "DUMP_RAM", "DUMP_EEPROM", "NOP",
//                               "READ_FILE", "read_temperature", "echo", "ENABLE_CLOCK",
//                               "DISABLE_CLOCK", "ENABLE_RESET", "DISABLE_RESET", "EXIT", "HELP",
//                               "SELECT_SLAVE", "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"
//                              };
//     const bool expected_return_values[] = {false, false, false, false, false, false, false, false,
//                                            false, false, false, false, false, false, false, false,
//                                            false, false, false, false, false, false, false, false,
//                                            false, false, false, false, false, false, false, false,
//                                            true, true, true, true, true, true, true, true, true, true
//                                           };
//     struct command_and_args cmd;

//     for (int idx = 0; idx < NUM_HW_COMMANDS + NUM_SW_COMMANDS; idx++) {
//         memset(&cmd, 0, sizeof(cmd));
//         char *command = (char *)commands[idx];
//         strcpy(cmd.command_name, command);
//         int expected_return_value = expected_return_values[idx];
//         int ret = is_expected_command_type(&cmd, "sw");
//         CHECK_EQUAL(ret, expected_return_value);
//     }
// }

// TEST(test_process_command_group, test_process_sw_command)
// {
//     struct command_and_args cmd;
//     cmd = {"INVALID_COMMAND", "INVALID_COMMAND", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_FAILED);
//     cmd = {"SELECT_SLAVE 0", "SELECT_SLAVE", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"SELECT_SLAVE", "SELECT_SLAVE", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"SELECT_SLAVE 2", "SELECT_SLAVE", {{"2"}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_FAILED);
//     cmd = {"SHOW_SLAVE", "SHOW_SLAVE", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"ENABLE_CLOCK", "ENABLE_CLOCK", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"DISABLE_CLOCK", "DISABLE_CLOCK", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     mock().expectOneCall("set_gpio_high").withParameter("pin", RESET_PIN).ignoreOtherParameters();
//     cmd = {"ENABLE_RESET", "ENABLE_RESET", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     mock().expectOneCall("set_gpio_low").withParameter("pin", RESET_PIN).ignoreOtherParameters();
//     cmd = {"DISABLE_RESET", "DISABLE_RESET", {{}}};
//     mock().expectOneCall("set_gpio_low").withParameter("pin", RESET_PIN).ignoreOtherParameters();
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"EXIT", "EXIT", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_EXIT);
//     cmd = {"HELP", "HELP", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"SET_CLK_FREQ", "SET_CLK_FREQ", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"SET_CLK_FREQ 5000", "SET_CLK_FREQ", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     cmd = {"SHOW_CLK_FREQ", "SHOW_CLK_FREQ", {{}}};
//     CHECK_EQUAL(process_sw_command(&cmd, NULL), SW_SUCCESS);
//     mock().checkExpectations();
// }
