#define BCM2835_PERI_BASE        0x3F000000			// peripheral base address
#define GPIO_BASE		(BCM2835_PERI_BASE + 0x200000) 	// GPIO controller base address

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

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

#define MAX_COMMAND_SIZE 256

int clk_enable = 0;
int clk_exit = 0;

struct mapping {
    char *command;
    char *binary;
};

struct mapping dict[] = {
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
    "ENABLE_CLOCK",  "000000",
    "DISABLE_CLOCK", "000000",
    "ENABLE_RESET",  "000000",
    "DISABLE_RESET", "000000",
    "EXIT",          "000000",
    "HELP",          "000000"
};

void print_help(void)
{
    printf("\nThe following literal operations are available:\n"
           "ADDLW\n"
           "ANDLW\n"
           "IORLW\n"
           "MOVLW\n"
           "SUBLW\n"
           "XORLW\n"
           "NOP\n\n"
           "These must be used as follows:\n"
           "<operation> <literal>\n\n"
           "The following byte-oriented operations are available:\n"
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
           "The following bit-oriented operations are available:\n"
           "BCF\n"
           "BSF\n\n"
           "These must be used as follows:\n"
           "<operation> <bit> <address>\n\n"
           "Other commands:\n"
           "READ_WREG\n"
           "READ_STATUS\n"
           "READ_ADDRESS\n"
           "DUMP_MEM\n"
           "ENABLE_CLOCK\n"
           "DISABLE_CLOCK\n"
           "ENABLE_RESET\n"
           "DISABLE_RESET\n"
           "EXIT\n\n");
    return;
}

void init_pins(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    int arr[] = {CLK_PIN, RESET_PIN, COMMAND_PIN, MOSI_PIN};
    size_t len = sizeof(arr)/sizeof(arr[0]);
    int idx = 0;
    while (idx < len) {
        INP_GPIO(arr[idx]); // Pin cannot be set as output unless first set as input
        OUT_GPIO(arr[idx]);
        idx = idx + 1;
    }
    INP_GPIO(RESULT_PIN);
    INP_GPIO(MISO_PIN);
}

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = 7; idx >= 0; idx--)
        result = result + data[idx] * (int)pow(2, 7-idx);

    return result;
}

bool get_command(char *key, char *value)
{
    int i = 0;
    char *name = dict[i].command;
    while (name) {
        if (strcmp(name, key) == 0) {
            memset(value, '\0', sizeof(value));
            strcpy(value, dict[i].binary);
            return true;
        }
        i++;
        name = dict[i].command;
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
                decimal_in = -1;
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

    gpio_map = mmap(
                    NULL,             	  // Any adddress in our space will do
                    BLOCK_SIZE,      	  // Map length
                    PROT_READ|PROT_WRITE, // Enable reading & writting to mapped memory
                    MAP_SHARED,       	  // Shared with other processes
                    mem_fd,          	  // File to map
                    GPIO_BASE        	  // Offset to GPIO peripheral
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

    while (timeout < 1000) {
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
            if (data_count < 8) {
                if (GET_GPIO(RESULT_PIN))
                    data[data_count] = 1;
                else
                    data[data_count] = 0;
            }
            data_count++;
        } else if (falling_check == 1 && GET_GPIO(CLK_PIN)) { // rising edge
            timeout++;
            falling_check = 0;
            if (data_count == 8) {
                result_decimal = binary_to_decimal(data);
                data_count = 0;
                printf("%d\n", result_decimal);
                result_decimal = 0;
                return NULL;
            }
        }
    }
    if (timeout >= 1000)
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

    while (timeout < 6000) {
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
            if (data_count == 1016) { 
                for (int idx = 126; idx >= 0; idx--) {
                    if (counter == 8) {
                        counter = 0;
                        printf("\n");
                    }
                    for (int i = 0; i < 8; i++)
                        byte_data[i] = data[idx*8 + i];
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
    if (timeout >= 6000)
        printf("Error occured with dumping memory\n");

    return NULL;
}

void *clk_thread(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;
  
    double clk_period = 1000000/CLK_FREQ_HZ;
    while(1) {
        if (clk_exit == 1)
            break;
        if (clk_enable == 1) {
            GPIO_SET = 1<<CLK_PIN;
            usleep(clk_period/2);
            GPIO_CLR = 1<<CLK_PIN;
            usleep(clk_period/2);
        }
    }

    return NULL;
}

int main(void)
{
    char binary_data_operand[99];
    char binary_data_opcode[99];
    char binary_data_bit_or_d[99];
    char binary_command[99];
    char instruction[99];
    char temp[99];
    int literal_or_address = 0;
    int bit_or_d = 0;
    int num_spaces = 0;
    pthread_t clk_thread_id;
    pthread_t read_result_thread_id;
    pthread_t mem_dump_thread_id;
    int operation;
    int falling_check = 0;
    volatile unsigned *gpio = init_gpio_map();
    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);

    printf("Please enter command, or \"HELP\" for instructions\n");
    while(1) {
        int i = 0;
        char *command = malloc(MAX_COMMAND_SIZE);
        fgets(command, MAX_COMMAND_SIZE, stdin);
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
            free(command);
            if (strcmp(instruction, "BCF") == 0 || strcmp(instruction, "BSF") == 0)
                decimal_to_binary(bit_or_d, binary_data_bit_or_d, 3);
            else
                decimal_to_binary(bit_or_d, binary_data_bit_or_d, 1);
            decimal_to_binary(literal_or_address, binary_data_operand, 7);
            if (!get_command(instruction, binary_data_opcode))
                continue;
            memset(temp, '\0', sizeof(temp));
            strcpy(temp, binary_data_opcode);
            strcat(temp, binary_data_bit_or_d);
            strcat(temp, binary_data_operand);
            memset(binary_command, '\0', sizeof(binary_command));
            strcpy(binary_command, temp);
        } else if (num_spaces == 1) { // literal instruction
            sscanf(command, "%s %d", instruction, &literal_or_address);
            free(command);
            decimal_to_binary(literal_or_address, binary_data_operand, 8);
            if (!get_command(instruction, binary_data_opcode))
                continue;
            memset(temp, '\0', sizeof(temp));
            strcpy(temp, binary_data_opcode);
            strcat(temp, binary_data_operand);
            memset(binary_command, '\0', sizeof(binary_command));
            strcpy(binary_command, temp);
        } else if (num_spaces == 0) {
            sscanf(command, "%s", instruction);
            free(command);
            literal_or_address = 0;
            decimal_to_binary(literal_or_address, binary_data_operand, 8);
            if (!get_command(instruction, binary_data_opcode))
                continue;
            memset(temp, '\0', sizeof(temp));
            strcpy(temp, binary_data_opcode);
            strcat(temp, binary_data_operand);
            memset(binary_command, '\0', sizeof(binary_command));
            strcpy(binary_command, temp);
        } else {
            printf("Invalid command\n");
            free(command);
            continue;
        }
        if (strcmp(instruction, "ENABLE_CLOCK") == 0) {
            clk_enable = 1;
            continue;
        } else if (strcmp(instruction, "DISABLE_CLOCK") == 0) {
            clk_enable = 0;
            continue;
        } else if (strcmp(instruction, "ENABLE_RESET") == 0) {
            GPIO_SET = 1<<RESET_PIN;
            continue;
        } else if (strcmp(instruction, "DISABLE_RESET") == 0) {
            GPIO_CLR = 1<<RESET_PIN;
            continue;
        } else if (strcmp(instruction, "EXIT") == 0) {
            GPIO_CLR = 1<<RESET_PIN;
            clk_exit = 1;
            break;
        } else if (strcmp(instruction, "HELP") == 0) {
            print_help();
            continue;
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
                        GPIO_SET = 1<<MOSI_PIN;
                    if (binary_command[idx] == '0')
                        GPIO_CLR = 1<<COMMAND_PIN;
                    else
                        GPIO_SET = 1<<COMMAND_PIN;
                } else {
                    GPIO_CLR = 1<<MOSI_PIN;
                }
                idx++;
            } else if (GET_GPIO(CLK_PIN) && falling_check == 1) { // rising edge
                falling_check = 0;
            }
        }
        if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
            strcmp(instruction, "READ_STATUS") == 0) {
            pthread_join(read_result_thread_id, NULL);
        } else if (strcmp(instruction, "DUMP_MEM") == 0)
            pthread_join(mem_dump_thread_id, NULL);
        i = 0;
        num_spaces = 0;
    }
    pthread_join(clk_thread_id, NULL);
    exit(0);
}
