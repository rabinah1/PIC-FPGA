#pragma once

#define MAX_INSTRUCTION_SIZE 32
#define CLK_PIN 5
#define CLK_IN_PIN 21
#define RESET_PIN 6
#define MISO_PIN 13
#define RESULT_PIN 16
#define MOSI_PIN 19
#define DATA_PIN 26
#define TIMER_EXT_CLK_PIN 25
#define MAX_STRING_SIZE 256
#define SLAVE_ID_FPGA 0
#define SLAVE_ID_ARDUINO 1
#define SW_SUCCESS 0
#define SW_FAILED 1
#define SW_EXIT -1
#define MAX_NUM_OF_ARGS 2
#define PROCESS_COMMAND_DELAY_USEC 200000

struct command_and_args {
    char full_command[MAX_STRING_SIZE];
    char command_name[MAX_STRING_SIZE];
    char command_args[MAX_NUM_OF_ARGS][MAX_STRING_SIZE];
};
