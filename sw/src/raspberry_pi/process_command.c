#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <regex.h>
#include "defines.h"
#include "common_data.h"
#include "hw_if.h"
#include "process_command.h"

#define NUM_SW_COMMANDS 10
#define NUM_HW_COMMANDS 32

static char hw_commands[NUM_HW_COMMANDS][MAX_STRING_SIZE] = {
    "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
    "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
    "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
    "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
    "READ_STATUS", "READ_ADDRESS", "DUMP_RAM", "DUMP_EEPROM",
    "NOP", "READ_FILE", "read_temperature", "echo"
};

static char sw_commands[NUM_SW_COMMANDS][MAX_STRING_SIZE] = {
    "ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET",
    "DISABLE_RESET", "EXIT", "HELP", "SELECT_SLAVE",
    "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"
};

static void print_help(void)
{
    printf("\nThere are two slaves: slave 0 (Terasic DE10-nano) and slave 1 (Arduino Nano)\n"
           "You can switch between these with command SELECT_SLAVE <slave_id>\n"
           "Slave 0 corresponds to the DE10-nano, and slave 1 corresponds to the Arduino\n"
           "You can check which slave is currently in use by SHOW_SLAVE command\n"
           "For slave 0, the following commands are available:\n"
           "Literal operations:\n"
           "ADDLW\n"
           "ANDLW\n"
           "IORLW\n"
           "MOVLW\n"
           "SUBLW\n"
           "XORLW\n"
           "NOP\n\n"
           "These must be used as follows:\n"
           "<operation> <literal>\n\n"
           "Byte-oriented operations:\n"
           "ADDWF\n"
           "ANDWF\n"
           "CLR\n"
           "COMF\n"
           "DECF\n"
           "DECFSZ\n"
           "INCF\n"
           "INCFSZ\n"
           "IORWF\n"
           "MOVF\n"
           "RLF\n"
           "RRF\n"
           "SUBWF\n"
           "SWAPF\n"
           "XORWF\n\n"
           "These must be used as follows:\n"
           "<operation> <d> <address>\n"
           "where <d> = 1 if result is stored to RAM\n"
           "and <d> = 0 if result is stored to W-register\n\n"
           "Bit-oriented operations:\n"
           "BCF\n"
           "BSF\n\n"
           "These must be used as follows:\n"
           "<operation> <bit> <address>\n\n"
           "Other commands:\n"
           "SHOW_SLAVE\n"
           "SELECT_SLAVE\n"
           "READ_WREG\n"
           "READ_STATUS\n"
           "READ_ADDRESS\n"
           "READ_FILE <file_name>\n"
           "DUMP_RAM\n"
           "DUMP_EEPROM\n"
           "ENABLE_CLOCK\n"
           "DISABLE_CLOCK\n"
           "ENABLE_RESET\n"
           "DISABLE_RESET\n"
           "SET_CLK_FREQ\n"
           "SHOW_CLK_FREQ\n"
           "EXIT\n\n"
           "For slave 1, the following operations are available:\n"
           "read_temperature\n"
           "echo <message>\n\n");
    return;
}

static void process_matched_command(struct command_and_args *cmd, size_t num_subexpr, char *command,
                                    regmatch_t *groups)
{
    int arg_idx = 0;
    char command_copy[strlen(command) + 1];
    memset(cmd->command_name, '\0', sizeof(cmd->command_name));
    memset(cmd->full_command, '\0', sizeof(cmd->full_command));

    for (int i = 0; i <= num_subexpr; i++) {
        memset(command_copy, '\0', sizeof(command_copy));
        strcpy(command_copy, command);

        if (groups[i].rm_so == (size_t) -1) {
            break;
        } else if (i == 0) { // Full string
            strcpy(cmd->full_command, command);
        } else if (i == 1) { // Command name
            command_copy[groups[i].rm_eo] = '\0';
            strcpy(cmd->command_name, command_copy + groups[i].rm_so);
        } else { // Command args
            command_copy[groups[i].rm_eo] = '\0';
            memset(cmd->command_args[arg_idx], '\0', sizeof(cmd->command_args[arg_idx]));
            strcpy(cmd->command_args[arg_idx], command_copy + groups[i].rm_so);
            arg_idx += 1;
        }
    }
}

static bool verify_command(char *command, struct command_and_args *cmd, int num_commands,
                           char *(*verifier)(int idx))
{
    regex_t compiled;
    regmatch_t *groups;
    char ref_regex[MAX_STRING_SIZE];
    size_t num_subexpr;
    size_t num_groups;
    bool match_found = false;

    for (int idx = 0; idx < num_commands; idx++) {
        memset(ref_regex, '\0', sizeof(ref_regex));
        strcpy(ref_regex, verifier(idx));

        if (regcomp(&compiled, ref_regex, REG_EXTENDED) != 0) {
            printf("%s, Failed to compile regex '%s'\n", __func__, ref_regex);
            return false;
        }

        num_subexpr = compiled.re_nsub; // Number of parenthesized subexpressions
        num_groups = num_subexpr + 1;
        groups = malloc(num_groups * sizeof(
                            regmatch_t)); // First item in groups is the full match of the string

        if (regexec(&compiled, command, num_groups, groups, 0) == 0) {
            match_found = true;
            process_matched_command(cmd, num_subexpr, command, groups);
        }

        free(groups);
        regfree(&compiled);

        if (match_found)
            return true;
    }

    return false;
}

bool verify_command_syntax(char *command, struct command_and_args *cmd)
{
    int num_commands_slave_0 = get_num_instructions_slave_0();
    int num_commands_slave_1 = get_num_instructions_slave_1();

    if (verify_command(command, cmd, num_commands_slave_0, get_slave_0_regex_command))
        return true;
    else if (verify_command(command, cmd, num_commands_slave_1, get_slave_1_regex_command))
        return true;

    return false;
}

bool is_expected_command_type(struct command_and_args *command, char *type)
{
    unsigned int num_expected_commands = 0;
    char (*expected_commands)[MAX_STRING_SIZE];

    if (strcmp(type, "sw") == 0) {
        expected_commands = malloc(NUM_SW_COMMANDS * MAX_STRING_SIZE);
        num_expected_commands = NUM_SW_COMMANDS;

        for (unsigned int i = 0; i < num_expected_commands; i++)
            strcpy(expected_commands[i], sw_commands[i]);
    } else if (strcmp(type, "hw") == 0) {
        expected_commands = malloc(NUM_HW_COMMANDS * MAX_STRING_SIZE);
        num_expected_commands = NUM_HW_COMMANDS;

        for (unsigned int i = 0; i < num_expected_commands; i++)
            strcpy(expected_commands[i], hw_commands[i]);
    } else {
        printf("%s, Invalid command type %s\n", __func__, type);
        return false;
    }

    for (unsigned int idx = 0; idx < num_expected_commands; idx++) {
        if (strcmp(command->command_name, expected_commands[idx]) == 0) {
            free(expected_commands);
            return true;
        }
    }

    free(expected_commands);
    return false;
}

static bool is_slave_valid(int *slave_id)
{
    if (*slave_id != SLAVE_ID_FPGA && *slave_id != SLAVE_ID_ARDUINO) {
        printf("%s, Invalid slave_id %d, setting to %d\n", __func__, *slave_id, SLAVE_ID_FPGA);
        *slave_id = SLAVE_ID_FPGA;
        return false;
    }

    return true;
}

int process_sw_command(struct command_and_args *command, volatile unsigned *gpio)
{
    char instruction[MAX_INSTRUCTION_SIZE];
    int slave_id = -1;
    int clk_freq = -1;
    memset(instruction, '\0', sizeof(instruction));

    if (strcmp(command->command_name, "SELECT_SLAVE") == 0) {
        slave_id = atoi(command->command_args[0]);

        if (!is_slave_valid(&slave_id))
            return SW_FAILED;

        set_slave_id(slave_id);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "SHOW_SLAVE") == 0) {
        printf("Slave %d\n", get_slave_id());
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "ENABLE_CLOCK") == 0) {
        set_clk_enable(true);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "DISABLE_CLOCK") == 0) {
        set_clk_enable(false);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "ENABLE_RESET") == 0) {
        set_gpio_high(RESET_PIN, gpio);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "DISABLE_RESET") == 0) {
        set_gpio_low(RESET_PIN, gpio);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "EXIT") == 0) {
        set_gpio_low(RESET_PIN, gpio);
        set_clk_exit(true);
        return SW_EXIT;
    } else if (strcmp(command->command_name, "HELP") == 0) {
        print_help();
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "SET_CLK_FREQ") == 0) {
        clk_freq = atoi(command->command_args[0]);
        set_clk_freq(clk_freq);
        return SW_SUCCESS;
    } else if (strcmp(command->command_name, "SHOW_CLK_FREQ") == 0) {
        printf("Clock frequency is %d\n", get_clk_freq());
        return SW_SUCCESS;
    }

    return SW_FAILED;
}

static bool process_one_fpga_command(struct command_and_args *command, volatile unsigned *gpio,
                                     FILE *result_file, bool write_to_file, char *serial_port)
{
    if (!send_command_to_hw(command, (void *)gpio, result_file, write_to_file,
                            serial_port)) {
        printf("%s, Sending command %s to HW failed\n", __func__, command->command_name);
        return false;
    }

    return true;
}

static bool process_one_line(struct command_and_args *command, char *line, volatile unsigned *gpio,
                             FILE *result_file, bool write_to_file, char *serial_port, FILE *input_file)
{
    memset(command, 0, sizeof(struct command_and_args));
    line[strlen(line) - 1] = '\0';

    if (verify_command_syntax(line, command)) {
        if (!send_command_to_hw(command, (void *)gpio, result_file, write_to_file,
                                serial_port)) {
            printf("%s, Sending command %s to HW failed\n", __func__, command->command_name);
            fclose(input_file);
            return false;
        }
    }

    usleep(PROCESS_COMMAND_DELAY_USEC);
    return true;
}

static bool process_commands_from_file(struct command_and_args *command, volatile unsigned *gpio,
                                       FILE *result_file, bool write_to_file, char *serial_port)
{
    char instruction[MAX_INSTRUCTION_SIZE];
    char filename[MAX_STRING_SIZE];
    char line[MAX_STRING_SIZE];
    memset(instruction, '\0', sizeof(instruction));
    memset(filename, '\0', sizeof(filename));
    memset(line, '\0', sizeof(line));
    strcpy(filename, command->command_args[0]);
    FILE *input_file = fopen(filename, "r");

    if (!input_file) {
        printf("%s, Failed to open file %s\n", __func__, filename);
        return false;
    }

    while (fgets(line, sizeof(line), input_file)) {
        line[strlen(line) - 1] = '\0'; // Remove trailing newline
    }

    rewind(input_file);

    while (fgets(line, sizeof(line), input_file)) {
        if (!process_one_line(command, line, gpio, result_file, write_to_file, serial_port, input_file))
            return false;
    }

    fclose(input_file);
    return true;
}

static bool process_fpga_command(struct command_and_args *command, volatile unsigned *gpio,
                                 FILE *result_file, bool write_to_file, char *serial_port)
{
    if (strcmp(command->command_name, "READ_FILE") != 0)
        return process_one_fpga_command(command, gpio, result_file, write_to_file, serial_port);
    else
        return process_commands_from_file(command, gpio, result_file, write_to_file, serial_port);
}

static bool process_arduino_command(struct command_and_args *command, volatile unsigned *gpio,
                                    FILE *result_file, bool write_to_file, char *serial_port)
{
    if (!send_command_to_hw(command, (void *)gpio, result_file, write_to_file,
                            serial_port)) {
        printf("%s, Sending command %s to HW failed\n", __func__, command->command_name);
        return false;
    }

    return true;
}

bool process_hw_command(struct command_and_args *command, volatile unsigned *gpio,
                        char *serial_port, FILE *result_file, bool write_to_file)
{
    if (get_slave_id() == SLAVE_ID_FPGA) {
        return process_fpga_command(command, gpio, result_file, write_to_file, serial_port);
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        return process_arduino_command(command, gpio, result_file, write_to_file, serial_port);
    } else {
        printf("%s, Slave %d is not valid, cannot process command\n", __func__, get_slave_id());
        return false;
    }
}
