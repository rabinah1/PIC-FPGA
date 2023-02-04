#include <string.h>
#include <stdint.h>
extern "C"
{
#include "hw_if.h"
#include "defines.h"
}
#include "../cpputest/include/CppUTest/TestHarness.h"
#include "../cpputest/include/CppUTestExt/MockSupport.h"

TEST_GROUP(test_hw_if_group)
{
    void teardown()
    {
        mock().clear();
    }
};

TEST(test_hw_if_group, test_convert_binary_to_decimal_nonzero)
{
    volatile int binary_data[8] = {1, 1, 0, 0, 0, 0, 1, 1};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 195);
}

TEST(test_hw_if_group, test_convert_binary_to_decimal_zero)
{
    volatile int binary_data[8] = {0};
    int decimal_data = binary_to_decimal(binary_data);
    CHECK_EQUAL(decimal_data, 0);
}

TEST(test_hw_if_group, test_convert_decimal_to_binary_8_bits)
{
    uint32_t decimal_data = 123;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "01111011";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(test_hw_if_group, test_convert_decimal_to_binary_8_bits_zero)
{
    uint32_t decimal_data = 0;
    int num_bits = 8;
    char binary_data[10] = {0};
    char correct_data[10] = "00000000";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(test_hw_if_group, test_convert_decimal_to_binary_16_bits)
{
    uint32_t decimal_data = 1110;
    int num_bits = 16;
    char binary_data[18] = {0};
    char correct_data[18] = "0000010001010110";
    decimal_to_binary(decimal_data, binary_data, num_bits);
    STRCMP_EQUAL(correct_data, binary_data);
}

TEST(test_hw_if_group, test_that_all_fpga_instructions_in_binary_are_found)
{
    char binary_data[10];
    memset(binary_data, '\0', sizeof(binary_data));
    const char *expected_commands[] = {
        "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF",
        "MOVF", "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW", "MOVLW",
        "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG", "READ_STATUS", "READ_ADDRESS", "DUMP_MEM",
        "NOP"};
    const char *expected_binary[] = {
        "000111", "000101", "000001", "001001", "000011", "001011", "001010", "001111",
        "000100", "001000", "001101", "001100", "000010", "001110", "000110", "111110",
        "111001", "111000", "110000", "111101", "111010", "0100", "0101", "110001", "110010",
        "110011", "101000", "000000"};
    int idx = 0;
    while (idx < 28) {
        char *command = (char *)expected_commands[idx];
        bool ret = get_command_in_binary(command, binary_data);
        CHECK(ret);
        STRCMP_EQUAL(expected_binary[idx], binary_data);
        idx++;
    }
}

TEST(test_hw_if_group, test_binary_command_creation)
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

TEST(test_hw_if_group, test_send_to_arduino_serial_connection_failed)
{
    char serial_port[] = "/dev/ttyUSB0";
    int baud_rate = 9600;
    mock().expectOneCall("serialOpen").withParameter("serial_port", serial_port).withParameter("baud_rate", baud_rate).andReturnValue(-1);
    int ret = send_to_arduino("read_temperature", NULL, "/dev/ttyUSB0");
    mock().checkExpectations();
    CHECK_EQUAL(1, ret);
}

TEST(test_hw_if_group, test_send_to_arduino_one_character)
{
    char serial_port[] = "/dev/ttyUSB0";
    int baud_rate = 9600;
    mock().expectOneCall("serialOpen").withParameter("serial_port", serial_port).withParameter("baud_rate", baud_rate).andReturnValue(0);
    mock().expectOneCall("serialPuts").withParameter("fd", 0).withParameter("command", "read_temperature");
    mock().expectOneCall("serialDataAvail").withParameter("fd", 0).andReturnValue(1);
    mock().expectOneCall("serialGetchar").withParameter("fd", 0).andReturnValue('\n');
    mock().expectOneCall("serialClose").withParameter("fd", 0);
    int ret = send_to_arduino("read_temperature", NULL, "/dev/ttyUSB0");
    mock().checkExpectations();
    CHECK_EQUAL(0, ret);
}

TEST(test_hw_if_group, test_send_to_arduino_multiple_characters)
{
    char serial_port[] = "/dev/ttyUSB0";
    int baud_rate = 9600;
    mock().expectOneCall("serialOpen").withParameter("serial_port", serial_port).withParameter("baud_rate", baud_rate).andReturnValue(0);
    mock().expectOneCall("serialPuts").withParameter("fd", 0).withParameter("command", "some_command");
    mock().expectOneCall("serialDataAvail").withParameter("fd", 0).andReturnValue(1);
    mock().expectOneCall("serialGetchar").withParameter("fd", 0).andReturnValue('a');
    mock().expectOneCall("serialDataAvail").withParameter("fd", 0).andReturnValue(1);
    mock().expectOneCall("serialGetchar").withParameter("fd", 0).andReturnValue('b');
    mock().expectOneCall("serialDataAvail").withParameter("fd", 0).andReturnValue(1);
    mock().expectOneCall("serialGetchar").withParameter("fd", 0).andReturnValue('\n');
    mock().expectOneCall("serialClose").withParameter("fd", 0);
    int ret = send_to_arduino("some_command", NULL, "/dev/ttyUSB0");
    mock().checkExpectations();
    CHECK_EQUAL(0, ret);
}
