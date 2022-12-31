#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include "defines.h"
#include "common_data.h"
#include "hw_if.h"

static char hw_commands[][MAX_STRING_SIZE] = {"ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ",
                                              "INCF", "INCFSZ", "IORWF", "MOVF", "RLF", "RRF",
                                              "SUBWF", "SWAPF", "XORWF", "ADDLW", "ANDLW", "IORLW",
                                              "MOVLW", "SUBLW", "XORLW", "BCF", "BSF", "READ_WREG",
                                              "READ_STATUS", "READ_ADDRESS", "DUMP_MEM", "NOP",
                                              "READ_FILE", "read_temperature", "echo"};
static char sw_commands[][MAX_STRING_SIZE] = {"ENABLE_CLOCK", "DISABLE_CLOCK", "ENABLE_RESET",
                                              "DISABLE_RESET", "EXIT", "HELP", "SELECT_SLAVE",
                                              "SHOW_SLAVE", "SET_CLK_FREQ", "SHOW_CLK_FREQ"};


void print_help(void)
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
           "DUMP_MEM\n"
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
                printf("%s, Failed to parse command %s\n", __func__, command);
                return false;
            }
        } else if (num_spaces == 1) { // literal instruction
            if (sscanf(command, "%s %d", instruction, &arg_1) != 2) {
                printf("%s, Failed to parse command %s\n", __func__, command);
                return false;
            }
        } else if (num_spaces == 0) {
            if (sscanf(command, "%s", instruction) != 1) {
                printf("%s, Failed to parse command %s\n", __func__, command);
                return false;
            }
        } else {
            printf("%s, Invalid number of spaces %d in command %s", __func__, num_spaces, command);
            return false;
        }

        int expected_num_args = get_expected_num_of_arguments(instruction);
        if (expected_num_args != num_spaces) {
            if (expected_num_args != -1)
                printf("%s, Invalid number of arguments %d for instruction %s\n", __func__,
                       num_spaces, instruction);
            return false;
        }
        return true;
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        return true;
    } else {
        printf("%s, Invalid slave_id %d\n", __func__, get_slave_id());
        return false;
    }
}

bool is_hw_command(char *command)
{
    char cmd[MAX_STRING_SIZE];
    unsigned int idx = 0;
    int num_hw_commands = sizeof(hw_commands) / sizeof(hw_commands[0]);
    memset(cmd, '\0', sizeof(char) * MAX_STRING_SIZE);
    
    while (command[idx] != '\0') {
        if (command[idx] == ' ' || command[idx] == '\n')
            break;
        idx++;
    }
    strncpy(cmd, command, idx);
    cmd[idx] = '\0';

    for (int idx = 0; idx < num_hw_commands; idx++) {
        char *test_command = (char *)hw_commands[idx];
        if (strcmp(cmd, test_command) == 0)
            return true;
    }

    return false;
}

bool is_sw_command(char *command)
{
    char cmd[MAX_STRING_SIZE];
    unsigned int idx = 0;
    int num_sw_commands = sizeof(sw_commands) / sizeof(sw_commands[0]);
    memset(cmd, '\0', sizeof(char) * MAX_STRING_SIZE);

    while (command[idx] != '\0') {
        if (command[idx] == ' ' || command[idx] == '\n')
            break;
        idx++;
    }
    strncpy(cmd, command, idx);
    cmd[idx] = '\0';

    for (int idx = 0; idx < num_sw_commands; idx++) {
        char *test_command = (char *)sw_commands[idx];
        if (strcmp(cmd, test_command) == 0)
            return true;
    }

    return false;
}

int process_sw_command(char *command, volatile unsigned *gpio)
{
    char cmd[MAX_STRING_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    unsigned int idx = 0;
    int slave_id = -1;
    int clk_freq = -1;
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
            printf("%s, Failed to parse command %s\n", __func__, command);
            return 1;
        } else {
            if (slave_id != SLAVE_ID_FPGA && slave_id != SLAVE_ID_ARDUINO) {
                printf("%s, Invalid slave_id %d, setting to %d\n", __func__, slave_id,
                       SLAVE_ID_FPGA);
                slave_id = SLAVE_ID_FPGA;
                return 1;
            }
            set_slave_id(slave_id);
            return 0;
        }
    } else if (strcmp(cmd, "SHOW_SLAVE") == 0) {
        printf("Slave %d\n", get_slave_id());
        return 0;
    } else if (strcmp(cmd, "ENABLE_CLOCK") == 0) {
        set_clk_enable(true);
        return 0;
    } else if (strcmp(cmd, "DISABLE_CLOCK") == 0) {
        set_clk_enable(false);
        return 0;
    } else if (strcmp(cmd, "ENABLE_RESET") == 0) {
        set_gpio_high(RESET_PIN, gpio);
        return 0;
    } else if (strcmp(cmd, "DISABLE_RESET") == 0) {
        set_gpio_low(RESET_PIN, gpio);
        return 0;
    } else if (strcmp(cmd, "EXIT") == 0) {
        set_gpio_low(RESET_PIN, gpio);
        set_clk_exit(true);
        return -1;
    } else if (strcmp(cmd, "HELP") == 0) {
        print_help();
        return 0;
    } else if (strcmp(cmd, "SET_CLK_FREQ") == 0) {
        if (sscanf(command, "%s %d", instruction, &clk_freq) != 2) {
            printf("%s, Failed to parse command %s\n", __func__, command);
            return 1;            
        } else {
            set_clk_freq(clk_freq);
            return 0;
        }
    } else if (strcmp(cmd, "SHOW_CLK_FREQ") == 0) {
        printf("Clock frequency is %d\n", get_clk_freq());
        return 0;
    }
    printf("%s, Command %s was not recognized\n", __func__, command);

    return 1;
}

bool process_hw_command(char *command, volatile unsigned *gpio, char *serial_port,
                        FILE *result_file, bool write_to_file)
{
    char cmd[MAX_STRING_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    char filename[MAX_STRING_SIZE];
    unsigned int idx = 0;
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
    memset(filename, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(cmd, '\0', sizeof(char) * MAX_STRING_SIZE);

    while (command[idx] != '\0') {
        if (command[idx] == ' ' || command[idx] == '\n')
            break;
        idx++;
    }
    strncpy(cmd, command, idx);
    cmd[idx] = '\0';

    if (get_slave_id() == SLAVE_ID_FPGA) {
        if (strcmp(cmd, "READ_FILE") != 0) {
            if (is_command_valid(command)) {
                if (!send_command_to_hw(command, (void *)gpio, result_file, write_to_file,
                                        serial_port)) {
                    printf("%s, Sending command %s to HW failed.\n", __func__, command);
                    return false;
                }
                return true;
            } else {
                return false;
            }
        } else {
            sscanf(command, "%s %s", instruction, filename);
            FILE *input_file = fopen(filename, "r");
            char line[MAX_STRING_SIZE];
            memset(line, '\0', sizeof(char) * MAX_STRING_SIZE);
            while (fgets(line, sizeof(line), input_file)) {
                line[strlen(line) - 1] = '\0'; // Remove trailing newline
                if (!is_command_valid(line)) {
                    printf("%s, Command %s in file %s was not valid.\n", __func__, line, filename);
                    return false;
                }
            }

            rewind(input_file);
            while (fgets(line, sizeof(line), input_file)) {
                line[strlen(line) - 1] = '\0';
                if (!send_command_to_hw(line, (void *)gpio, result_file, write_to_file,
                                        serial_port)) {
                    printf("%s, Sending command %s to HW failed.\n", __func__, command);
                    return false;
                }
                usleep(200000);
            }
            return true;
            fclose(input_file);
        }
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        if (is_command_valid(command)) {
            if (!send_command_to_hw(command, (void *)gpio, result_file, write_to_file,
                                    serial_port)) {
                printf("%s, Sending command %s to HW failed.\n", __func__, command);
                return false;
            }
            return true;
        }
        printf("%s, Command %s is not valid.\n", __func__, command);
        return false;
    } else {
        printf("%s, Slave %d is not valid, cannot process command.\n", __func__, get_slave_id());
        return false;
    }
}
