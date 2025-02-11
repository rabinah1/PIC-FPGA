#include <stdio.h>
#include "defines.h"

void set_gpio_high(int pin, volatile unsigned *gpio);
void set_gpio_low(int pin, volatile unsigned *gpio);
int binary_to_decimal(volatile int *data);
void decimal_to_binary(uint32_t decimal_in, char *binary_out, int num_bits);
bool get_command_in_binary(char *instruction, char *opcode);
bool create_binary_command(struct command_and_args *command, char *binary_command,
                           char *instruction);
void *clk_thread(void *arguments);
void *timer_ext_clk_thread(void *arguments);
bool send_command_to_hw(struct command_and_args *command, void *arguments, FILE *result_file,
                        bool write_to_file,
                        char *serial_port);
bool send_command_to_arduino(struct command_and_args *command, FILE *result_file,
                             char *serial_port);
