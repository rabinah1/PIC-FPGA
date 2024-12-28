#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/poll.h>
#include <errno.h>
#include <math.h>
#ifndef UNIT_TEST
#include <wiringSerial.h>
#else
#include "mock_wiringSerial.h"
#endif
#include "hw_if.h"
#include "defines.h"
#include "common_data.h"

#define ARDUINO_SERIAL_STABILIZE_SEC 2
#define NUM_OTHER_INSTRUCTIONS 5
#define NUM_LITERAL_INSTRUCTIONS 7
#define NUM_BIT_OR_BYTE_INSTRUCTIONS 17
#define POLL_GPIO (POLLPRI | POLLERR)
#define NUM_BITS_EEPROM 2048
#define NUM_BITS_RAM 1016
#define NUM_BYTES_EEPROM 256
#define NUM_BYTES_RAM 127
#define MEM_DUMP_VALUES_PER_LINE 8
#define DATA_BIT_WIDTH 8
#define MEM_DUMP_TIMEOUT 12000
#define RESULT_TIMEOUT 1000
#define GPIO_CLR *(gpio+10)
#define GPIO_SET *(gpio+7)
#define TIMER_EXT_CLK_FREQ_HZ 1
#define BINARY_COMMAND_SIZE 14
#define MAX_OPCODE_SIZE 6
#define MAX_OPERAND_SIZE 8
#define MAX_BIT_OR_D_SIZE 3
#define BAUD_RATE 9600
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define unlikely(x) __builtin_expect(!!(x), 0)

struct result_thread_args {
    volatile unsigned *gpio;
    FILE *result_file;
    bool write_to_file;
};

struct mem_dump_thread_args {
    volatile unsigned *gpio;
    int num_bits;
    int num_bytes;
};

static char bit_or_byte_instructions[NUM_BIT_OR_BYTE_INSTRUCTIONS][MAX_STRING_SIZE] = {
    "ADDWF", "ANDWF", "CLR", "COMF", "DECF", "DECFSZ", "INCF", "INCFSZ", "IORWF", "MOVF",
    "RLF", "RRF", "SUBWF", "SWAPF", "XORWF", "BCF", "BSF"
};

static char literal_instructions[NUM_LITERAL_INSTRUCTIONS][MAX_STRING_SIZE] = {
    "ADDLW", "ANDLW", "IORLW", "MOVLW", "SUBLW", "XORLW", "READ_ADDRESS"
};

static char other_instructions[NUM_OTHER_INSTRUCTIONS][MAX_STRING_SIZE] = {
    "READ_WREG", "READ_STATUS", "DUMP_RAM", "DUMP_EEPROM", "NOP"
};

#ifndef UNIT_TEST
void set_gpio_high(int pin, volatile unsigned *gpio)
{
    GPIO_SET = 1 << pin;
}

void set_gpio_low(int pin, volatile unsigned *gpio)
{
    GPIO_CLR = 1 << pin;
}
#endif

int binary_to_decimal(volatile int *data)
{
    int result = 0;

    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, DATA_BIT_WIDTH - 1 - idx);

    return result;
}

void decimal_to_binary(uint32_t decimal_in, char *binary_out, int num_bits)
{
    bool zero_flag = false;

    for (int idx = num_bits - 1; idx >= 0; idx--) {
        if ((uint32_t)pow(2, idx) > decimal_in || zero_flag) {
            binary_out[num_bits - 1 - idx] = '0';
        } else {
            binary_out[num_bits - 1 - idx] = '1';
            decimal_in = decimal_in - (uint32_t)pow(2, idx);

            if (decimal_in == 0)
                zero_flag = true;
        }
    }

    binary_out[num_bits] = '\0';
}

bool get_command_in_binary(char *instruction, char *opcode)
{
    int num_instructions = get_num_instructions_slave_0();

    for (int idx = 0; idx < num_instructions; idx++) {
        if (strcmp(get_slave_0_command(idx), instruction) == 0) {
            strcpy(opcode, get_slave_0_binary(idx));
            return true;
        }
    }

    printf("%s, Invalid instruction %s\n", __func__, instruction);
    return false;
}

static bool create_bit_or_byte_oriented_instruction(struct command_and_args *command,
        char *instruction,
        char *binary_command)
{
    uint32_t bit_or_d = atoi(command->command_args[0]);
    uint32_t literal_or_address = atoi(command->command_args[1]);
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        return false;
    }

    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    char *binary_data_bit_or_d = malloc(MAX_BIT_OR_D_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_bit_or_d, '\0', sizeof(char) * MAX_BIT_OR_D_SIZE);

    if (strcmp(instruction, "BCF") == 0 || strcmp(instruction, "BSF") == 0)
        decimal_to_binary(bit_or_d, binary_data_bit_or_d, 3);
    else
        decimal_to_binary(bit_or_d, binary_data_bit_or_d, 1);

    decimal_to_binary(literal_or_address, binary_data_operand, 7);
    sprintf(binary_command, "%s%s%s", binary_data_opcode, binary_data_bit_or_d,
            binary_data_operand);
    free(binary_data_opcode);
    free(binary_data_operand);
    free(binary_data_bit_or_d);
    return true;
}

static bool create_literal_instruction(struct command_and_args *command, char *instruction,
                                       char *binary_command)
{
    uint32_t literal_or_address = atoi(command->command_args[0]);
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        return false;
    }

    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    decimal_to_binary(literal_or_address, binary_data_operand, 8);
    sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    free(binary_data_opcode);
    free(binary_data_operand);
    return true;
}

static bool create_other_instruction(struct command_and_args *command, char *instruction,
                                     char *binary_command)
{
    uint32_t literal_or_address = 0;
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        return false;
    }

    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    decimal_to_binary(literal_or_address, binary_data_operand, 8);
    sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    free(binary_data_opcode);
    free(binary_data_operand);
    return true;
}

static bool is_bit_or_byte_instruction(struct command_and_args *command)
{
    for (unsigned int i = 0; i < NUM_BIT_OR_BYTE_INSTRUCTIONS; i++) {
        if (strcmp(command->command_name, bit_or_byte_instructions[i]) == 0)
            return true;
    }

    return false;
}

static bool is_literal_instruction(struct command_and_args *command)
{
    for (unsigned int i = 0; i < NUM_LITERAL_INSTRUCTIONS; i++) {
        if (strcmp(command->command_name, literal_instructions[i]) == 0)
            return true;
    }

    return false;
}

static bool is_other_instruction(struct command_and_args *command)
{
    for (unsigned int i = 0; i < NUM_OTHER_INSTRUCTIONS; i++) {
        if (strcmp(command->command_name, other_instructions[i]) == 0)
            return true;
    }

    return false;
}

bool create_binary_command(struct command_and_args *command, char *binary_command,
                           char *instruction)
{
    if (is_bit_or_byte_instruction(command)) { // Bit-oriented or byte-oriented instruction
        return create_bit_or_byte_oriented_instruction(command, instruction, binary_command);
    } else if (is_literal_instruction(command)) { // Literal instruction
        return create_literal_instruction(command, instruction, binary_command);
    } else if (is_other_instruction(command)) { // Other instruction
        return create_other_instruction(command, instruction, binary_command);
    } else {
        printf("%s, Invalid command %s\n", __func__, command->command_name);
        return false;
    }
}

static void print_result(volatile int *data, bool write_to_file, FILE *result_file)
{
    int result_decimal = binary_to_decimal(data);

    if (write_to_file)
        fprintf(result_file, "%d\n", result_decimal);
    else
        printf("%d\n", result_decimal);
}

static void *result_thread(void *arguments)
{
    struct result_thread_args *args = (struct result_thread_args *)arguments;
    volatile unsigned *gpio = args->gpio;
    volatile int data[DATA_BIT_WIDTH] = {0};
    FILE *result_file = args->result_file;
    bool write_to_file = args->write_to_file;
    bool miso_trigger = false;
    int data_count = 0;
    int timeout = 0;
    int poll_ret = 0;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE];
    sprintf(clk_in_pin_file, "/sys/class/gpio/gpio%d/value", CLK_IN_PIN);
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);

    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio %d\n", __func__, CLK_IN_PIN);
        return NULL;
    }

    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (timeout < RESULT_TIMEOUT) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        poll_ret = poll(pfds, 1, poll_timeout);

        if (poll_ret == 0) {
            printf("%s, Waiting for clock edge timed out\n", __func__);
            close(clk_in_pin_fd);
            return NULL;
        } else if (poll_ret < 0) {
            printf("%s, Error %d happened for poll in thread result_thread\n", __func__, errno);
            close(clk_in_pin_fd);
            return NULL;
        } else if (poll_ret > 0 && !(GET_GPIO(CLK_PIN)) && (pfds[0].revents & POLL_GPIO)) {
            // falling edge
            timeout++;

            if (!miso_trigger) {
                miso_trigger = GET_GPIO(MISO_PIN) ? true : false;
            } else {
                if (data_count < DATA_BIT_WIDTH)
                    data[data_count] = GET_GPIO(RESULT_PIN) ? 1 : 0;

                data_count++;
            }
        } else if (poll_ret > 0 && GET_GPIO(CLK_PIN) && (pfds[0].revents & POLL_GPIO)) {
            // rising edge
            timeout++;

            if (data_count == DATA_BIT_WIDTH) {
                print_result(data, write_to_file, result_file);
                close(clk_in_pin_fd);
                return NULL;
            }
        }
    }

    if (timeout >= RESULT_TIMEOUT)
        printf("%s, Error occured when executing instruction\n", __func__);

    close(clk_in_pin_fd);
    return NULL;
}

static void *mem_dump_thread(void *arguments)
{
    struct mem_dump_thread_args *args = (struct mem_dump_thread_args *)arguments;
    volatile unsigned *gpio = args->gpio;
    int num_bits = args->num_bits;
    int num_bytes = args->num_bytes;
    volatile int data[NUM_BITS_EEPROM] = {0};
    bool miso_trigger = false;
    int byte_data[DATA_BIT_WIDTH] = {0};
    int data_count = 0;
    int result = 0;
    int counter = 0;
    int timeout = 0;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE];
    sprintf(clk_in_pin_file, "/sys/class/gpio/gpio%d/value", CLK_IN_PIN);
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);

    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio %d\n", __func__, CLK_IN_PIN);
        return NULL;
    }

    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (timeout < MEM_DUMP_TIMEOUT) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        int poll_ret = poll(pfds, 1, poll_timeout);

        if (poll_ret == 0) {
            printf("%s, Waiting for clock edge timed out\n", __func__);
            close(clk_in_pin_fd);
            return NULL;
        } else if (poll_ret < 0) {
            printf("%s, Error %d happened for poll in thread mem_dump_thread\n", __func__, errno);
            close(clk_in_pin_fd);
            return NULL;
        } else if (poll_ret > 0 && !(GET_GPIO(CLK_PIN)) && (pfds[0].revents & POLL_GPIO)) {
            // falling edge
            timeout++;

            if (!miso_trigger) {
                miso_trigger = GET_GPIO(MISO_PIN) ? true : false;
            } else {
                if (data_count < num_bits)
                    data[data_count] = GET_GPIO(RESULT_PIN) ? 1 : 0;

                data_count++;
            }
        } else if (poll_ret > 0 && GET_GPIO(CLK_PIN) && (pfds[0].revents & POLL_GPIO)) {
            // rising edge
            timeout++;

            if (data_count == num_bits) {
                for (int idx = num_bytes - 1; idx >= 0; idx--) {
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
                close(clk_in_pin_fd);
                return NULL;
            }
        }
    }

    if (timeout >= MEM_DUMP_TIMEOUT)
        printf("%s, Error occured with dumping memory\n", __func__);

    close(clk_in_pin_fd);
    return NULL;
}

void *clk_thread(void *arguments)
{
    volatile unsigned *gpio;
    double clk_period = 1000000 / get_clk_freq();
    useconds_t clk_half_period = clk_period / 2;
    gpio = (volatile unsigned *)arguments;

    while (true) {
        if (unlikely(clk_period != 1000000 / get_clk_freq())) {
            clk_period = 1000000 / get_clk_freq();
            clk_half_period = clk_period / 2;
        }

        if (unlikely(get_clk_exit()))
            break;

        if (get_clk_enable()) {
            set_gpio_high(CLK_PIN, gpio);
            usleep(clk_half_period);
            set_gpio_low(CLK_PIN, gpio);
            usleep(clk_half_period);
        }
    }

    return NULL;
}

void *timer_ext_clk_thread(void *arguments)
{
    volatile unsigned *gpio;
    double timer_ext_clk_period = 1000000 / TIMER_EXT_CLK_FREQ_HZ;
    useconds_t timer_ext_clk_half_period = timer_ext_clk_period / 2;
    gpio = (volatile unsigned *)arguments;

    while (true) {
        if (unlikely(get_clk_exit()))
            break;

        if (get_clk_enable()) {
            set_gpio_high(TIMER_EXT_CLK_PIN, gpio);
            usleep(timer_ext_clk_half_period);
            set_gpio_low(TIMER_EXT_CLK_PIN, gpio);
            usleep(timer_ext_clk_half_period);
        }
    }

    return NULL;
}

static bool open_serial_connection(int *fd, char *serial_port)
{
    if ((*fd = serialOpen(serial_port, BAUD_RATE)) < 0) {
        printf("%s, Failed to open serial connection to port %s, exiting...\n", __func__, serial_port);
        return false;
    }

    sleep(ARDUINO_SERIAL_STABILIZE_SEC); // Wait for serial connection to stabilize
    return true;
}

static void receive_response(int fd, FILE *result_file)
{
    char input = '\0';

    while (true) {
        if (serialDataAvail(fd) > 0) {
            input = (char)serialGetchar(fd);

            if (result_file != NULL)
                fprintf(result_file, "%c", input);
            else
                printf("%c", input);

            if (input == '\n')
                break;
        }
    }
}

bool send_command_to_arduino(struct command_and_args *command, FILE *result_file, char *serial_port)
{
    int fd;

    if (!open_serial_connection(&fd, serial_port))
        return false;

    serialPuts(fd, command->full_command);
    receive_response(fd, result_file);
    serialClose(fd);
    printf("\n%s, Connection closed\n", __func__);
    return true;
}

static bool send_to_fpga(char *binary_command, volatile unsigned *gpio)
{
    char clk_in_pin_file[MAX_STRING_SIZE];
    sprintf(clk_in_pin_file, "/sys/class/gpio/gpio%d/value", CLK_IN_PIN);
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);

    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio %d\n", __func__, CLK_IN_PIN);
        return false;
    }

    // binary_command = <opcode_in_binary> + <argument_in_binary>
    // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
    size_t length = strlen(binary_command);
    size_t idx = 0;
    int poll_ret = 0;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    struct pollfd pfds[1];
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (idx <= length) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        poll_ret = poll(pfds, 1, poll_timeout);

        if (poll_ret == 0) {
            printf("%s, Waiting for clock edge timed out\n", __func__);
            close(clk_in_pin_fd);
            return false;
        } else if (poll_ret < 0) {
            printf("%s, Error %d happened for poll\n", __func__, errno);
            close(clk_in_pin_fd);
            return false;
        } else if (poll_ret > 0 && !(GET_GPIO(CLK_PIN)) && (pfds[0].revents & POLL_GPIO)) {
            // falling edge
            if (idx < length) {
                if (idx == 0)
                    set_gpio_high(MOSI_PIN, gpio);

                if (binary_command[idx] == '0')
                    set_gpio_low(DATA_PIN, gpio);
                else
                    set_gpio_high(DATA_PIN, gpio);
            } else {
                set_gpio_low(MOSI_PIN, gpio);
            }

            idx++;
        }
    }

    close(clk_in_pin_fd);
    return true;
}

static bool send_command_to_fpga(void *arguments, struct command_and_args *command,
                                 FILE *result_file, bool write_to_file)
{
    bool send_status = true;
    volatile unsigned *gpio;
    struct result_thread_args result_args;
    struct mem_dump_thread_args mem_dump_args;
    char binary_command[BINARY_COMMAND_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    pthread_t read_result_thread_id;
    pthread_t mem_dump_thread_id;
    memset(binary_command, '\0', sizeof(binary_command));
    memset(instruction, '\0', sizeof(instruction));
    gpio = (volatile unsigned *)arguments;
    result_args.gpio = gpio;
    result_args.result_file = result_file;
    result_args.write_to_file = write_to_file;
    mem_dump_args.gpio = gpio;

    if (!create_binary_command(command, binary_command, instruction)) {
        printf("%s, Could not create binary command from command %s\n", __func__, command->command_name);
        return false;
    }

    if (!get_clk_enable()) {
        printf("%s, Clock is not enabled, please enable it first\n", __func__);
        return false;
    }

    if (strcmp(instruction, "READ_WREG") == 0 ||
        strcmp(instruction, "READ_ADDRESS") == 0 ||
        strcmp(instruction, "READ_STATUS") == 0) {
        pthread_create(&read_result_thread_id, NULL, result_thread, (void *)&result_args);
        usleep(1000);
    } else if (strcmp(instruction, "DUMP_RAM") == 0 ||
               strcmp(instruction, "DUMP_EEPROM") == 0) {
        if (strcmp(instruction, "DUMP_RAM") == 0) {
            mem_dump_args.num_bits = NUM_BITS_RAM;
            mem_dump_args.num_bytes = NUM_BYTES_RAM;
        } else {
            mem_dump_args.num_bits = NUM_BITS_EEPROM;
            mem_dump_args.num_bytes = NUM_BYTES_EEPROM;
        }

        pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)&mem_dump_args);
        usleep(1000);
    }

    if (!send_to_fpga(binary_command, gpio))
        send_status = false;

    if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
        strcmp(instruction, "READ_STATUS") == 0)
        pthread_join(read_result_thread_id, NULL);
    else if (strcmp(instruction, "DUMP_RAM") == 0 ||
             strcmp(instruction, "DUMP_EEPROM") == 0)
        pthread_join(mem_dump_thread_id, NULL);

    return send_status;
}

bool send_command_to_hw(struct command_and_args *command, void *arguments, FILE *result_file,
                        bool write_to_file, char *serial_port)
{
    if (get_slave_id() == SLAVE_ID_FPGA) {
        return send_command_to_fpga(arguments, command, result_file, write_to_file);
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        return send_command_to_arduino(command, result_file, serial_port);
    } else {
        printf("%s, Invalid slave_id %d, cannot process command \"%s\"\n", __func__,
               get_slave_id(), command->command_name);
        return false;
    }
}
