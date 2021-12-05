#define MAX_OPERAND_SIZE 8
#define MAX_OPCODE_SIZE 6
#define MAX_BIT_OR_D_SIZE 3
#define BINARY_COMMAND_SIZE 14
#define MAX_INSTRUCTION_SIZE 32

int binary_to_decimal(volatile int *data);
void decimal_to_binary(int decimal_in, char *binary_out, int num_bits);
bool instruction_exists(char *instruction);
bool get_command(char *instruction, char *opcode);
int create_binary_command(char *command, char *binary_command, char *instruction,
                          char *binary_data_operand, char *binary_data_opcode,
                          char *binary_data_bit_or_d);
