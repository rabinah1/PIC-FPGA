#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>
#include "code.h"
#include "types.h"

#define DATA_BIT_WIDTH 8

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, 7 - idx);

    return result;
}

void decimal_to_binary(int decimal_in, char *binary_out, int num_bits)
{
    int idx = num_bits - 1;
    memset(binary_out, '\0', sizeof(char) * (uint32_t)num_bits);
    while (idx >= 0) {
        if ((int)pow(2, idx) > decimal_in) {
            binary_out[num_bits - 1 - idx] = '0';
        } else {
            binary_out[num_bits - 1 - idx] = '1';
            decimal_in = decimal_in - (int)pow(2, idx);
            if (decimal_in == 0)
                decimal_in--;
        }
        idx--;
    }
    binary_out[num_bits] = '\0';
    
}

bool instruction_exists(char *instruction)
{
    int num_instructions = sizeof(slave_0_commands) / sizeof(slave_0_commands[0]);
    int i = 0;
    char *name;
    while (i < num_instructions) {
        name = slave_0_commands[i].command;
        if (strcmp(name, instruction) == 0)
            return true;
        i++;
    }

    printf("Instruction %s does not exist\n", instruction);
    return false;
}

bool get_command(char *instruction, char *opcode)
{
    int i = 0;
    int num_instructions = sizeof(slave_0_commands) / sizeof(slave_0_commands[0]);
    char *name;
    while (i < num_instructions) {
        name = slave_0_commands[i].command;
        if (strcmp(name, instruction) == 0) {
            memset(opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
            strcpy(opcode, slave_0_commands[i].binary);
            return true;
        }
        i++;
    }

    printf("Invalid command\n");
    return false;
}

int create_binary_command(char *command, char *binary_command, char *instruction,
                          char *binary_data_operand, char *binary_data_opcode,
                          char *binary_data_bit_or_d)
{
    int literal_or_address = 0;
    int bit_or_d = 0;
    int num_spaces = 0;
    int i = 0;
    while (command[i] != '\0') {
        if (command[i] == ' ')
            num_spaces++;
        i++;
    }

    memset(binary_data_bit_or_d, '\0', sizeof(char) * MAX_BIT_OR_D_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    memset(binary_command, '\0', sizeof(char) * BINARY_COMMAND_SIZE);
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
    if (num_spaces == 2) { // bit-oriented or byte-oriented instruction
        sscanf(command, "%s %d %d", instruction, &bit_or_d, &literal_or_address);
        if (strcmp(instruction, "BCF") == 0 || strcmp(instruction, "BSF") == 0)
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 3);
        else
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 1);
        decimal_to_binary(literal_or_address, binary_data_operand, 7);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_bit_or_d);
        strcat(binary_command, binary_data_operand);

    } else if (num_spaces == 1) { // literal instruction
        sscanf(command, "%s %d", instruction, &literal_or_address);
        decimal_to_binary((uint32_t)literal_or_address, binary_data_operand, 8);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_operand);

    } else if (num_spaces == 0) {
        sscanf(command, "%s", instruction);
        literal_or_address = 0;
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_operand);

    } else {
        printf("Invalid command\n");
        return 0;
    }
    return 0;
}
