int binary_to_decimal(volatile int *data);
void decimal_to_binary(int decimal_in, char *binary_out, int num_bits);
bool instruction_exists(char *instruction);
int get_expected_num_of_arguments(char *instruction);
bool get_command(char *instruction, char *opcode);
bool create_binary_command(char *command, char *binary_command, char *instruction,
                           char *binary_data_operand, char *binary_data_opcode,
                           char *binary_data_bit_or_d);
