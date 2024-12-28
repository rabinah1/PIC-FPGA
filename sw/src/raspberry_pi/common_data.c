#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "defines.h"
#include "common_data.h"

#define INVALID_NUM_ARGS -1
#define CLK_FREQ_DEFAULT 4000

struct command_to_binary {
    char *command;
    char *binary;
};

static int slave_id = SLAVE_ID_FPGA;
static bool clk_enable = false;
static bool clk_exit = false;
static int clk_freq = CLK_FREQ_DEFAULT;
static char slave_1_commands_regex[][MAX_STRING_SIZE] = {"^(read_temperature)$", "^(echo)\\s+(.*)$"};
static char slave_0_commands_regex[][MAX_STRING_SIZE] = {
    "^(ADDWF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(ANDWF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(CLR)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(COMF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(DECF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(DECFSZ)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(INCF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(INCFSZ)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(IORWF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(MOVF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(RLF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(RRF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(SUBWF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(SWAPF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(XORWF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(ADDLW)\\s+([0-9]+)$",
    "^(ANDLW)\\s+([0-9]+)$",
    "^(IORLW)\\s+([0-9]+)$",
    "^(MOVLW)\\s+([0-9]+)$",
    "^(SUBLW)\\s+([0-9]+)$",
    "^(XORLW)\\s+([0-9]+)$",
    "^(BCF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(BSF)\\s+([0-9]+)\\s+([0-9]+)$",
    "^(READ_WREG)$",
    "^(READ_STATUS)$",
    "^(READ_ADDRESS)\\s+([0-9]+)$",
    "^(DUMP_RAM)$",
    "^(DUMP_EEPROM)$",
    "^(NOP)$",
    "^(READ_FILE)\\s+(\\S+)$",
    "^(ENABLE_CLOCK)$",
    "^(DISABLE_CLOCK)$",
    "^(ENABLE_RESET)$",
    "^(DISABLE_RESET)$",
    "^(EXIT)$",
    "^(HELP)$",
    "^(SELECT_SLAVE)\\s+([0-9])$",
    "^(SHOW_SLAVE)$",
    "^(SET_CLK_FREQ)\\s+([0-9]+)$",
    "^(SHOW_CLK_FREQ)$"
};
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
    {"DUMP_RAM",      "101000"},
    {"DUMP_EEPROM",   "101100"},
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

char *get_slave_0_regex_command(int idx)
{
    return slave_0_commands_regex[idx];
}

char *get_slave_1_regex_command(int idx)
{
    return slave_1_commands_regex[idx];
}
