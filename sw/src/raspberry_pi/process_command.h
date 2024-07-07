#include <stdio.h>

void print_help(void);
bool verify_command_syntax(char *command, struct command_and_args *cmd);
bool is_command_valid(char *command);
bool is_expected_command_type(struct command_and_args *command, char *type);
int process_sw_command(struct command_and_args *command, volatile unsigned *gpio);
bool process_hw_command(struct command_and_args *command, volatile unsigned *gpio,
                        char *serial_port,
                        FILE *result_file, bool write_to_file);
