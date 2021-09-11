#define BCM2835_PERI_BASE 0x3F000000 // peripheral base address
#define GPIO_BASE (BCM2835_PERI_BASE + 0x200000) // GPIO controller base address

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>
#include <wiringPi.h>
#include <wiringSerial.h>

#define BLOCK_SIZE (4*20) // only using gpio registers region
#define CLK_FREQ_HZ 4000
#define CLK_PIN 5
#define RESET_PIN 6
#define MISO_PIN 13
#define RESULT_PIN 16
#define MOSI_PIN 19
#define COMMAND_PIN 26
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define MAX_OPERAND_SIZE 8
#define MAX_OPCODE_SIZE 6
#define MAX_BIT_OR_D_SIZE 3
#define BINARY_COMMAND_SIZE 14
#define MAX_INSTRUCTION_SIZE 32
#define MAX_STRING_SIZE 256
#define EXIT_COMMAND -1
#define RESULT_TIMEOUT 1000
#define MEM_DUMP_TIMEOUT 6000
#define DATA_BIT_WIDTH 8
#define MEM_DUMP_VALUES_PER_LINE 8
#define NUM_BYTES_RAM 127
#define NUM_BITS_RAM 1016

int clk_enable = 0;
int clk_exit = 0;
char serial_port[MAX_STRING_SIZE];

struct mapping {
    char *command;
    char *binary;
};

struct mapping slave_0_commands[] = {
    "ADDWF",         "000111",
    "ANDWF",         "000101",
    "CLR",           "000001",
    "COMF",          "001001",
    "DECF",          "000011",
    "DECFSZ",        "001011",
    "INCF",          "001010",
    "INCFSZ",        "001111",
    "IORWF",         "000100",
    "MOVF",          "001000",
    "RLF",           "001101",
    "RRF",           "001100",
    "SUBWF",         "000010",
    "SWAPF",         "001110",
    "XORWF",         "000110",
    "ADDLW",         "111110",
    "ANDLW",         "111001",
    "IORLW",         "111000",
    "MOVLW",         "110000",
    "SUBLW",         "111101",
    "XORLW",         "111010",
    "BCF",           "0100",
    "BSF",           "0101",
    "READ_WREG",     "110001",
    "READ_STATUS",   "110010",
    "READ_ADDRESS",  "110011",
    "DUMP_MEM",      "101000",
    "NOP",           "000000",
    "READ_FILE",     "000000",
    "ENABLE_CLOCK",  "000000",
    "DISABLE_CLOCK", "000000",
    "ENABLE_RESET",  "000000",
    "DISABLE_RESET", "000000",
    "EXIT",          "000000",
    "HELP",          "000000",
    "SELECT_SLAVE",  "000000",
    "SHOW_SLAVE",    "000000"
};

char slave_1_commands[][MAX_STRING_SIZE] = {"read_temperature"};

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
           "READ_WREG\n"
           "READ_STATUS\n"
           "READ_ADDRESS\n"
           "READ_FILE <file_name>\n"
           "DUMP_MEM\n"
           "ENABLE_CLOCK\n"
           "DISABLE_CLOCK\n"
           "ENABLE_RESET\n"
           "DISABLE_RESET\n"
           "EXIT\n\n"
           "For slave 1, the following operations are available:\n"
           "read_temperature\n\n");
    return;
}

void init_pins(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    int arr[] = {CLK_PIN, RESET_PIN, COMMAND_PIN, MOSI_PIN};
    size_t len = sizeof(arr) / sizeof(arr[0]);
    int idx = 0;
    while (idx < len) {
        INP_GPIO(arr[idx]); // Pin cannot be set as output unless first set as input
        OUT_GPIO(arr[idx]);
        idx++;
    }
    INP_GPIO(RESULT_PIN);
    INP_GPIO(MISO_PIN);
}

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, 7 - idx);

    return result;
}

bool instruction_exists(char *key)
{
    int num_instructions = sizeof(slave_0_commands) / sizeof(slave_0_commands[0]);
    int i = 0;
    char *name;
    while (i < num_instructions) {
        name = slave_0_commands[i].command;
        if (strcmp(name, key) == 0)
            return true;
        i++;
    }

    printf("Instruction %s does not exist\n", key);
    return false;
}

bool get_command(char *key, char *value)
{
    int i = 0;
    int num_instructions = sizeof(slave_0_commands) / sizeof(slave_0_commands[0]);
    char *name;
    while (i < num_instructions) {
        name = slave_0_commands[i].command;
        if (strcmp(name, key) == 0) {
            memset(value, '\0', sizeof(value));
            strcpy(value, slave_0_commands[i].binary);
            return true;
        }
        i++;
    }

    printf("Invalid command\n");
    return false;
}

void decimal_to_binary(int decimal_in, char *binary_out, int num_bits)
{
    int idx = num_bits - 1;
    memset(binary_out, '\0', sizeof(binary_out));
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
}

volatile unsigned* init_gpio_map(void)
{
    // open file for mapping
    int mem_fd;
    void *gpio_map;
    volatile unsigned *gpio;
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("can't open /dev/mem \n");
        exit(-1);
    }

    gpio_map = mmap(NULL,                 // Any adddress in our space will do
                    BLOCK_SIZE,           // Map length
                    PROT_READ|PROT_WRITE, // Enable reading & writting to mapped memory
                    MAP_SHARED,           // Shared with other processes
                    mem_fd,               // File to map
                    GPIO_BASE             // Offset to GPIO peripheral
                    );
    close(mem_fd);

    if (gpio_map == MAP_FAILED) {
        printf("mmap error %d\n", (int)gpio_map);
        exit(-1);
    }
    gpio = (volatile unsigned *)gpio_map;

    return gpio;
}

void *result_thread(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    volatile int data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int miso_trigger = 0;
    int falling_check = 0;
    int data_count = 0;
    int result_decimal = 0;
    int timeout = 0;

    while (timeout < RESULT_TIMEOUT) {
        if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && miso_trigger == 0) { // falling edge, miso_trigger not received
            timeout++;
            falling_check = 1;
            if (GET_GPIO(MISO_PIN))
                miso_trigger = 1;
            else
                miso_trigger = 0;
        } else if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && miso_trigger == 1) { // falling edge, miso_trigger received
            timeout++;
            falling_check = 1;
            if (data_count < DATA_BIT_WIDTH) {
                if (GET_GPIO(RESULT_PIN))
                    data[data_count] = 1;
                else
                    data[data_count] = 0;
            }
            data_count++;
        } else if (falling_check == 1 && GET_GPIO(CLK_PIN)) { // rising edge
            timeout++;
            falling_check = 0;
            if (data_count == DATA_BIT_WIDTH) {
                result_decimal = binary_to_decimal(data);
                data_count = 0;
                printf("%d\n", result_decimal);
                result_decimal = 0;
                return NULL;
            }
        }
    }
    if (timeout >= RESULT_TIMEOUT)
        printf("Error occured when executing instruction\n");

    return NULL;
}

void *mem_dump_thread(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    volatile int data[1016] = {0};
    int miso_trigger = 0;
    int byte_data[8] = {0};
    int falling_check = 0;
    int data_count = 0;
    int result_decimal = 0;
    int result = 0;
    int counter = 0;
    int timeout = 0;

    while (timeout < MEM_DUMP_TIMEOUT) {
        if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && miso_trigger == 0) { // falling edge, miso_trigger not received
            timeout++;
            falling_check = 1;
            if (GET_GPIO(MISO_PIN))
                miso_trigger = 1;
            else
                miso_trigger = 0;
        } else if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && miso_trigger == 1) { // falling edge, miso_trigger received
            timeout++;
            falling_check = 1;
            if (data_count < 1016) {
                if (GET_GPIO(RESULT_PIN))
                    data[data_count] = 1;
                else
                    data[data_count] = 0;
            }
            data_count++;
        } else if (falling_check == 1 && GET_GPIO(CLK_PIN)) { // rising edge
            timeout++;
            falling_check = 0;
            if (data_count == NUM_BITS_RAM) {
                for (int idx = NUM_BYTES_RAM - 1; idx >= 0; idx--) {
                    if (counter == MEM_DUMP_VALUES_PER_LINE) {
                        counter = 0;
                        printf("\n");
                    }
                    for (int i = 0; i < DATA_BIT_WIDTH; i++)
                        byte_data[i] = data[idx * DATA_BIT_WIDTH + i];
                    result = binary_to_decimal(byte_data);
                    printf("%3d   ", result);
                    counter++;
                }
                printf("\n");
                data_count = 0;
                return NULL;
            }
        }
    }
    if (timeout >= MEM_DUMP_TIMEOUT)
        printf("Error occured with dumping memory\n");

    return NULL;
}

void *clk_thread(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    double clk_period = 1000000 / CLK_FREQ_HZ;
    while(1) {
        if (clk_exit == 1)
            break;
        if (clk_enable == 1) {
            GPIO_SET = 1 << CLK_PIN;
            usleep(clk_period / 2);
            GPIO_CLR = 1 << CLK_PIN;
            usleep(clk_period / 2);
        }
    }

    return NULL;
}

bool check_validity(char *command)
{
    int i = 0;
    int num_spaces = 0;
    int bit_or_d;
    int literal_or_address = 0;
    char binary_data_bit_or_d[MAX_BIT_OR_D_SIZE];
    char binary_data_operand[MAX_OPERAND_SIZE];
    char binary_data_opcode[MAX_OPCODE_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    while (command[i] != '\0') {
        if (command[i] == ' ')
            num_spaces++;
        i++;
    }

    memset(binary_data_bit_or_d, '\0', sizeof(binary_data_bit_or_d));
    memset(binary_data_operand, '\0', sizeof(binary_data_operand));
    memset(binary_data_opcode, '\0', sizeof(binary_data_opcode));
    memset(instruction, '\0', sizeof(instruction));
    if (num_spaces == 2) // bit- or byte-oriented instruction
        sscanf(command, "%s %d %d", instruction, &bit_or_d, &literal_or_address);
    else if (num_spaces == 1) // literal instruction
        sscanf(command, "%s %d", instruction, &literal_or_address);
    else if (num_spaces == 0)
        sscanf(command, "%s", instruction);
    else
        return false;
    if (!instruction_exists(instruction))
        return false;

    return true;
}

int process_command(char *command, void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;
    char binary_data_operand[MAX_OPERAND_SIZE];
    char binary_data_opcode[MAX_OPCODE_SIZE];
    char binary_data_bit_or_d[MAX_BIT_OR_D_SIZE];
    char binary_command[BINARY_COMMAND_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    int literal_or_address = 0;
    int bit_or_d = 0;
    int num_spaces = 0;
    pthread_t read_result_thread_id;
    pthread_t mem_dump_thread_id;
    int falling_check = 0;
    int i = 0;
    while (command[i] != '\0') {
        if (command[i] == ' ')
            num_spaces++;
        i++;
    }

    memset(binary_data_bit_or_d, '\0', sizeof(binary_data_bit_or_d));
    memset(binary_data_operand, '\0', sizeof(binary_data_operand));
    memset(binary_data_opcode, '\0', sizeof(binary_data_opcode));
    if (num_spaces == 2) { // bit-oriented or byte-oriented instruction
        sscanf(command, "%s %d %d", instruction, &bit_or_d, &literal_or_address);
        if (strcmp(instruction, "BCF") == 0 || strcmp(instruction, "BSF") == 0)
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 3);
        else
            decimal_to_binary(bit_or_d, binary_data_bit_or_d, 1);
        decimal_to_binary(literal_or_address, binary_data_operand, 7);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        memset(binary_command, '\0', sizeof(binary_command));
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_bit_or_d);
        strcat(binary_command, binary_data_operand);

    } else if (num_spaces == 1) { // literal instruction
        sscanf(command, "%s %d", instruction, &literal_or_address);
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        memset(binary_command, '\0', sizeof(binary_command));
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_operand);

    } else if (num_spaces == 0) {
        sscanf(command, "%s", instruction);
        literal_or_address = 0;
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        if (!get_command(instruction, binary_data_opcode))
            return 0;
        memset(binary_command, '\0', sizeof(binary_command));
        strcpy(binary_command, binary_data_opcode);
        strcat(binary_command, binary_data_operand);

    } else {
        printf("Invalid command\n");
        return 0;
    }

    if (strcmp(instruction, "ENABLE_CLOCK") == 0) {
        clk_enable = 1;
        return 0;
    } else if (strcmp(instruction, "DISABLE_CLOCK") == 0) {
        clk_enable = 0;
        return 0;
    } else if (strcmp(instruction, "ENABLE_RESET") == 0) {
        GPIO_SET = 1<<RESET_PIN;
        return 0;
    } else if (strcmp(instruction, "DISABLE_RESET") == 0) {
        GPIO_CLR = 1<<RESET_PIN;
        return 0;
    } else if (strcmp(instruction, "EXIT") == 0) {
        GPIO_CLR = 1<<RESET_PIN;
        clk_exit = 1;
        return EXIT_COMMAND;
    } else if (strcmp(instruction, "HELP") == 0) {
        print_help();
        return 0;
    } else if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
               strcmp(instruction, "READ_STATUS") == 0) {
        pthread_create(&read_result_thread_id, NULL, result_thread, (void *)gpio);
        usleep(1000);
    } else if (strcmp(instruction, "DUMP_MEM") == 0) {
        pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)gpio);
        usleep(1000);
    }
    // binary_command = <opcode_in_binary> + <argument_in_binary>
    // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
    int length = strlen(binary_command);
    int idx = 0;
    while (idx <= length) {
        if (!(GET_GPIO(CLK_PIN)) && falling_check == 0) { // falling edge
            falling_check = 1;
            if (idx < length) {
                if (idx == 0)
                    GPIO_SET = 1 << MOSI_PIN;
                if (binary_command[idx] == '0')
                    GPIO_CLR = 1 << COMMAND_PIN;
                else
                    GPIO_SET = 1 << COMMAND_PIN;
            } else {
                GPIO_CLR = 1 << MOSI_PIN;
            }
            idx++;
        } else if (GET_GPIO(CLK_PIN) && falling_check == 1) { // rising edge
            falling_check = 0;
        }
    }
    if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
        strcmp(instruction, "READ_STATUS") == 0)
        pthread_join(read_result_thread_id, NULL);
    else if (strcmp(instruction, "DUMP_MEM") == 0)
        pthread_join(mem_dump_thread_id, NULL);
    return 0;
}

int send_to_arduino(char *command)
{
    int fd;
    if (wiringPiSetup() < 0)
        return 1;
    if ((fd = serialOpen(serial_port, 9600)) < 0)
        return 1;
    usleep(2000000); // Wait for serial connection to stabilize
    serialPuts(fd, command);

    while (1) {
        if (serialDataAvail(fd) > 0) {
            char input = serialGetchar(fd);
            if (input == '\n')
                break;
            printf("%c", input);
        }
    }
    serialClose(fd);
    printf("\nConnection closed\n");
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("Exactly one command line argument is required.\n");
        return 0;
    }
    strcpy(serial_port, argv[1]);
    bool is_valid = true;
    char filename[MAX_STRING_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    int slave_sel = 0;
    pthread_t clk_thread_id;
    volatile unsigned *gpio = init_gpio_map();
    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);

    printf("Please enter command, or \"HELP\" for instructions\n");
    while(1) {
        memset(filename, '\0', sizeof(filename));
        char *command = malloc(MAX_STRING_SIZE);
        fgets(command, MAX_STRING_SIZE, stdin);
        if (strstr(command, "SELECT_SLAVE") != NULL) {
            if (check_validity(command))
                sscanf(command, "%s %d", instruction, &slave_sel);

        } else if (strstr(command, "SHOW_SLAVE") != NULL) {
            if (check_validity(command))
                printf("Slave %d\n", slave_sel);

        } else if (slave_sel == 0) { // Slave is FPGA
            if (strstr(command, "READ_FILE") == NULL) { // READ_FILE not found
                if (check_validity(command)) {
                    if (process_command(command, (void *)gpio) == EXIT_COMMAND) {
                        free(command);
                        break;
                    }
                }
            } else { // READ_FILE found
                sscanf(command, "%s %s", instruction, filename);
                FILE *f = fopen(filename, "r");
                char line[MAX_STRING_SIZE];
                is_valid = true;
                while (fgets(line, sizeof(line), f)) {
                    line[strlen(line) - 1] = '\0'; // Remove trailing newline
                    if (!check_validity(line)) {
                        is_valid = false;
                        break;
                    }
                }
                if (is_valid) {
                    rewind(f);
                    while (fgets(line, sizeof(line), f)) {
                        line[strlen(line) - 1] = '\0';
                        process_command(line, (void *)gpio);
                        usleep(100000);
                    }
                }
            }

        } else if (slave_sel == 1) { // Slave is Arduino
            command[strlen(command) - 1] = '\0';
            bool command_found = false;
            int num_instructions = sizeof(slave_1_commands) / sizeof(slave_1_commands[0]);
            for (int i = 0; i < num_instructions; i++) {
                if (strcmp(slave_1_commands[i], command) == 0) {
                    command_found = true;
                    break;
                }
            }
            if (command_found)
                send_to_arduino(command);
            else
                printf("Command \"%s\" is not valid for slave 1\n", command);
        }
        free(command);
    }
    pthread_join(clk_thread_id, NULL);
    exit(0);
}
