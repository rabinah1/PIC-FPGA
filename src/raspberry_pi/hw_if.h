#include <stdio.h>

void set_gpio_high(int pin, volatile unsigned *gpio);
void set_gpio_low(int pin, volatile unsigned *gpio);
int binary_to_decimal(volatile int *data);
void decimal_to_binary(uint32_t decimal_in, char *binary_out, int num_bits);
bool get_command_in_binary(char *instruction, char *opcode);
bool create_binary_command(char *command, char *binary_command, char *instruction);
void *result_thread(void *arguments);
void *mem_dump_thread(void *arguments);
void *clk_thread(void *arguments);
void *timer_ext_clk_thread(void *arguments);
bool send_command_to_fpga(void *arguments, char *command, FILE *result_file, bool write_to_file);
bool send_command_to_hw(char *command, void *arguments, FILE *result_file, bool write_to_file,
                        char *serial_port);
bool send_command_to_arduino(char *command, FILE *result_file, char *serial_port);
