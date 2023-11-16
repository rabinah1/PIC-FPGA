#include <stdio.h>

void print_help(void);
bool is_command_valid(char *command);
bool is_expected_command_type(char *command, char *type);
int process_sw_command(char *command, volatile unsigned *gpio);
bool process_hw_command(char *command, volatile unsigned *gpio, char *serial_port,
                        FILE *result_file, bool write_to_file);
