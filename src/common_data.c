#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "defines.h"
#include "common_data.h"

struct command_to_binary {
    char *command;
    char *binary;
};

struct command_to_num_args {
    char *command;
    int num_args;
};

static int slave_id = SLAVE_ID_FPGA;
static bool clk_enable = false;
static bool clk_exit = false;
static int clk_freq = CLK_FREQ_DEFAULT;
static char slave_1_commands[][MAX_STRING_SIZE] = {"read_temperature", "echo"};
static struct command_to_binary slave_0_commands[] = {
    {"ADDWF",         "000111"},
    {"ANDWF",         "000101"},
    {"CLR",           "000001"},
    {"COMF",          "001001"},
    {"DECF",          "000011"},
    {"DECFSZ",        "001011"},
    {"INCF",          "001010"},
    {"INCFSZ",        "001111"},
    {"IORWF",         "000100"},
    {"MOVF",          "001000"},
    {"RLF",           "001101"},
    {"RRF",           "001100"},
    {"SUBWF",         "000010"},
    {"SWAPF",         "001110"},
    {"XORWF",         "000110"},
    {"ADDLW",         "111110"},
    {"ANDLW",         "111001"},
    {"IORLW",         "111000"},
    {"MOVLW",         "110000"},
    {"SUBLW",         "111101"},
    {"XORLW",         "111010"},
    {"BCF",           "0100"},
    {"BSF",           "0101"},
    {"READ_WREG",     "110001"},
    {"READ_STATUS",   "110010"},
    {"READ_ADDRESS",  "110011"},
    {"DUMP_MEM",      "101000"},
    {"NOP",           "000000"},
    {"READ_FILE",     "None"},
    {"ENABLE_CLOCK",  "None"},
    {"DISABLE_CLOCK", "None"},
    {"ENABLE_RESET",  "None"},
    {"DISABLE_RESET", "None"},
    {"EXIT",          "None"},
    {"HELP",          "None"},
    {"SELECT_SLAVE",  "None"},
    {"SHOW_SLAVE",    "None"},
    {"SET_CLK_FREQ",  "None"},
    {"SHOW_CLK_FREQ", "None"}
};
static struct command_to_num_args slave_0_commands_to_args[] = {
    {"ADDWF",         2},
    {"ANDWF",         2},
    {"CLR",           2},
    {"COMF",          2},
    {"DECF",          2},
    {"DECFSZ",        2},
    {"INCF",          2},
    {"INCFSZ",        2},
    {"IORWF",         2},
    {"MOVF",          2},
    {"RLF",           2},
    {"RRF",           2},
    {"SUBWF",         2},
    {"SWAPF",         2},
    {"XORWF",         2},
    {"ADDLW",         1},
    {"ANDLW",         1},
    {"IORLW",         1},
    {"MOVLW",         1},
    {"SUBLW",         1},
    {"XORLW",         1},
    {"BCF",           2},
    {"BSF",           2},
    {"READ_WREG",     0},
    {"READ_STATUS",   0},
    {"READ_ADDRESS",  1},
    {"DUMP_MEM",      0},
    {"NOP",           0},
    {"READ_FILE",     1},
    {"ENABLE_CLOCK",  0},
    {"DISABLE_CLOCK", 0},
    {"ENABLE_RESET",  0},
    {"DISABLE_RESET", 0},
    {"EXIT",          0},
    {"HELP",          0},
    {"SELECT_SLAVE",  1},
    {"SHOW_SLAVE",    0},
    {"SET_CLK_FREQ",  0},
    {"SHOW_CLK_FREQ", 0}
};

void set_clk_freq(int value)
{
    clk_freq = value;
}

int get_clk_freq(void)
{
    return clk_freq;
}

void set_clk_exit(bool value)
{
    clk_exit = value;
}

bool get_clk_exit(void)
{
    return clk_exit;
}

void set_clk_enable(bool value)
{
    clk_enable = value;
}

bool get_clk_enable(void)
{
    return clk_enable;
}

int get_slave_id(void)
{
    return slave_id;
}

void set_slave_id(int id)
{
    slave_id = id;
}

int get_num_instructions_slave_0(void)
{
    return sizeof(slave_0_commands) / sizeof(slave_0_commands[0]);
}

int get_num_instructions_slave_1(void)
{
    return sizeof(slave_1_commands) / sizeof(slave_1_commands[0]);
}

char *get_slave_0_binary(int idx)
{
    return slave_0_commands[idx].binary;
}

char *get_slave_0_command(int idx)
{
    return slave_0_commands[idx].command;
}

char *get_slave_1_command(int idx)
{
    return slave_1_commands[idx];
}

int get_expected_num_of_arguments(char *instruction)
{
    int num_instructions = get_num_instructions_slave_0();
    int idx = 0;
    while (idx < num_instructions) {
        if (strcmp(slave_0_commands_to_args[idx].command, instruction) == 0)
            return slave_0_commands_to_args[idx].num_args;
        idx++;
    }

    printf("%s, Instruction %s does not exist\n", __func__, instruction);
    return INVALID_NUM_ARGS;
}
