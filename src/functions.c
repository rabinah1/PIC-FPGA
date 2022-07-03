#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "functions.h"
#include "defines.h"

struct mapping {
    char *command;
    char *binary;
};

struct num_args_mapping {
    char *command;
    int num_args;
};

static int slave_id = SLAVE_ID_FPGA;
static char slave_1_commands[][MAX_STRING_SIZE] = {"read_temperature", "echo"};

static struct mapping slave_0_commands[] = {
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
    {"READ_FILE",     "000000"},
    {"ENABLE_CLOCK",  "000000"},
    {"DISABLE_CLOCK", "000000"},
    {"ENABLE_RESET",  "000000"},
    {"DISABLE_RESET", "000000"},
    {"EXIT",          "000000"},
    {"HELP",          "000000"},
    {"SELECT_SLAVE",  "000000"},
    {"SHOW_SLAVE",    "000000"}
};

static struct num_args_mapping slave_0_commands_to_args[] = {
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
    {"SHOW_SLAVE",    0}
};

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

char *get_slave_0_command(int idx)
{
    return slave_0_commands[idx].command;
}

char *get_slave_1_command(int idx)
{
    return slave_1_commands[idx];
}

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, 7 - idx);

    return result;
}

void decimal_to_binary(uint32_t decimal_in, char *binary_out, int num_bits)
{
    int idx = num_bits - 1;
    bool zero_flag = false;
    memset(binary_out, '\0', sizeof(char) * (uint32_t)num_bits);
    while (idx >= 0) {
        if ((uint32_t)pow(2, idx) > decimal_in || zero_flag) {
            binary_out[num_bits - 1 - idx] = '0';
        } else {
            binary_out[num_bits - 1 - idx] = '1';
            decimal_in = decimal_in - (uint32_t)pow(2, idx);
            if (decimal_in == 0)
                zero_flag = true;
        }
        idx--;
    }
    binary_out[num_bits] = '\0';
}

int get_expected_num_of_arguments(char *instruction)
{
    int num_instructions = get_num_instructions_slave_0();
    int idx = 0;
    char *name;
    while (idx < num_instructions) {
        name = slave_0_commands_to_args[idx].command;
        if (strcmp(name, instruction) == 0)
            return slave_0_commands_to_args[idx].num_args;
        idx++;
    }

    printf("Instruction %s does not exist\n", instruction);
    return -1;
}

bool get_command_in_binary(char *instruction, char *opcode)
{
    int idx = 0;
    int num_instructions = get_num_instructions_slave_0();
    char *name;
    while (idx < num_instructions) {
        name = get_slave_0_command(idx);
        if (strcmp(name, instruction) == 0) {
            memset(opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
            strcpy(opcode, slave_0_commands[idx].binary);
            return true;
        }
        idx++;
    }

    printf("Invalid instruction %s\n", instruction);
    return false;
}

bool create_binary_command(char *command, char *binary_command, char *instruction)
{
    char *binary_data_operand = (char *)malloc(MAX_OPERAND_SIZE);
    char *binary_data_opcode = (char *)malloc(MAX_OPCODE_SIZE);
    char *binary_data_bit_or_d = (char *)malloc(MAX_BIT_OR_D_SIZE);
    uint32_t literal_or_address = 0;
    uint32_t bit_or_d = 0;
    int num_spaces = 0;
    int idx = 0;

    while (command[idx] != '\0') {
        if (command[idx] == ' ')
            num_spaces++;
        idx++;
    }
    memset(binary_data_bit_or_d, '\0', sizeof(char) * MAX_BIT_OR_D_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    memset(binary_command, '\0', sizeof(char) * BINARY_COMMAND_SIZE);
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
    if (num_spaces == 2) { // bit-oriented or byte-oriented instruction
        sscanf(command, "%s %d %d", instruction, &bit_or_d, &literal_or_address);
        if (!get_command_in_binary(instruction, binary_data_opcode)) {
            free(binary_data_opcode);
            free(binary_data_operand);
            free(binary_data_bit_or_d);
            return false;
        }
        if (strcmp(instruction, "BCF") == 0 || strcmp(instruction, "BSF") == 0)
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 3);
        else
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 1);
        decimal_to_binary(literal_or_address, binary_data_operand, 7);
        sprintf(binary_command, "%s%s%s", binary_data_opcode, binary_data_bit_or_d, binary_data_operand);
    } else if (num_spaces == 1) { // literal instruction
        sscanf(command, "%s %d", instruction, &literal_or_address);
        if (!get_command_in_binary(instruction, binary_data_opcode)) {
            free(binary_data_opcode);
            free(binary_data_operand);
            free(binary_data_bit_or_d);
            return false;
        }
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    } else if (num_spaces == 0) {
        sscanf(command, "%s", instruction);
        if (!get_command_in_binary(instruction, binary_data_opcode)) {
            free(binary_data_opcode);
            free(binary_data_operand);
            free(binary_data_bit_or_d);
            return false;
        }
        literal_or_address = 0;
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    } else {
        printf("Invalid number of spaces %d in command %s\n", num_spaces, command);
        free(binary_data_opcode);
        free(binary_data_operand);
        free(binary_data_bit_or_d);
        return false;
    }
    free(binary_data_opcode);
    free(binary_data_operand);
    free(binary_data_bit_or_d);
    return true;
}

int handle_slave_commands(char *command)
{
    char cmd[MAX_STRING_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    unsigned int idx = 0;
    memset(cmd, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);

    while (command[idx] != '\0') {
        if (command[idx] == ' ' || command[idx] == '\n')
            break;
        idx++;
    }
    strncpy(cmd, command, idx);
    cmd[idx] = '\0';

    if (strcmp(cmd, "SELECT_SLAVE") == 0) {
        if (sscanf(command, "%s %d", instruction, &slave_id) != 2) {
            printf("Failed to parse command %s\n", command);
            return 1;
        } else {
            if (slave_id != SLAVE_ID_FPGA && slave_id != SLAVE_ID_ARDUINO) {
                printf("Invalid slave_id %d, setting to 0\n", slave_id);
                slave_id = 0;
                return 1;
            }
            return 0;
        }
    } else if (strcmp(cmd, "SHOW_SLAVE") == 0) {
        printf("Slave %d\n", slave_id);
        return 0;
    }

    return -1;
}

bool is_command_valid(char *command)
{
    if (get_slave_id() == SLAVE_ID_FPGA) {
        int idx = 0;
        int num_spaces = 0;
        int arg_1 = 0;
        int arg_2 = 0;
        char instruction[MAX_INSTRUCTION_SIZE];

        while (command[idx] != '\0') {
            if (command[idx] == ' ')
                num_spaces++;
            idx++;
        }
        memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
        if (num_spaces == 2) { // bit- or byte-oriented instruction
            if (sscanf(command, "%s %d %d", instruction, &arg_1, &arg_2) != 3) {
                printf("Failed to parse command %s\n", command);
                return false;
            }
        } else if (num_spaces == 1) { // literal instruction
            if (sscanf(command, "%s %d", instruction, &arg_1) != 2) {
                printf("Failed to parse command %s\n", command);
                return false;
            }
        } else if (num_spaces == 0) {
            if (sscanf(command, "%s", instruction) != 1) {
                printf("Failed to parse command %s\n", command);
                return false;
            }
        } else {
            printf("Invalid number of spaces %d in command %s", num_spaces, command);
            return false;
        }

        int expected_num_args = get_expected_num_of_arguments(instruction);
        if (expected_num_args != num_spaces) {
            if (expected_num_args != -1)
                printf("Invalid number of arguments %d for instruction %s\n", num_spaces, instruction);
            return false;
        }
        return true;
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        return true;
    } else {
        printf("Invalid slave_id %d\n", get_slave_id());
        return false;
    }
}
