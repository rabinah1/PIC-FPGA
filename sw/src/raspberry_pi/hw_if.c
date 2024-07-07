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
    int idx = num_bits - 1;
    bool zero_flag = false;

    while (idx >= 0) {
        if ((uint32_t)pow(2, idx) > decimal_in || zero_flag) {
            binary_out[num_bits - 1 - idx] = '0';
        } else {
            binary_out[num_bits - 1 - idx] = '1';
            decimal_in = decimal_in - (uint32_t)pow(2, idx);

            if (decimal_in == 0)
                zero_flag = true;
        }

        idx--;
    }

    binary_out[num_bits] = '\0';
}

bool get_command_in_binary(char *instruction, char *opcode)
{
    int idx = 0;
    int num_instructions = get_num_instructions_slave_0();

    while (idx < num_instructions) {
        if (strcmp(get_slave_0_command(idx), instruction) == 0) {
            strcpy(opcode, get_slave_0_binary(idx));
            return true;
        }

        idx++;
    }

    printf("%s, Invalid instruction %s\n", __func__, instruction);
    return false;
}

bool create_bit_or_byte_oriented_instruction(struct command_and_args *command, char *instruction,
        char *binary_command)
{
    uint32_t bit_or_d = 0;
    uint32_t literal_or_address = 0;
    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    char *binary_data_bit_or_d = malloc(MAX_BIT_OR_D_SIZE);
    memset(binary_data_bit_or_d, '\0', sizeof(char) * MAX_BIT_OR_D_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);
    bit_or_d = atoi(command->command_args[0]);
    literal_or_address = atoi(command->command_args[1]);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        free(binary_data_operand);
        free(binary_data_bit_or_d);
        return false;
    }

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

bool create_literal_instruction(struct command_and_args *command, char *instruction,
                                char *binary_command)
{
    uint32_t literal_or_address = 0;
    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);
    literal_or_address = atoi(command->command_args[0]);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        free(binary_data_operand);
        return false;
    }

    decimal_to_binary(literal_or_address, binary_data_operand, 8);
    sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    free(binary_data_opcode);
    free(binary_data_operand);
    return true;
}

bool create_other_instruction(struct command_and_args *command, char *instruction,
                              char *binary_command)
{
    uint32_t literal_or_address = 0;
    char *binary_data_operand = malloc(MAX_OPERAND_SIZE);
    char *binary_data_opcode = malloc(MAX_OPCODE_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    strcpy(instruction, command->command_name);

    if (!get_command_in_binary(instruction, binary_data_opcode)) {
        free(binary_data_opcode);
        free(binary_data_operand);
        return false;
    }

    literal_or_address = 0;
    decimal_to_binary(literal_or_address, binary_data_operand, 8);
    sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    free(binary_data_opcode);
    free(binary_data_operand);
    return true;
}

bool is_bit_or_byte_instruction(struct command_and_args *command)
{
    for (unsigned int i = 0; i < NUM_BIT_OR_BYTE_INSTRUCTIONS; i++) {
        if (strcmp(command->command_name, bit_or_byte_instructions[i]) == 0)
            return true;
    }

    return false;
}

bool is_literal_instruction(struct command_and_args *command)
{
    for (unsigned int i = 0; i < NUM_LITERAL_INSTRUCTIONS; i++) {
        if (strcmp(command->command_name, literal_instructions[i]) == 0)
            return true;
    }

    return false;
}

bool is_other_instruction(struct command_and_args *command)
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

void print_result(volatile int *data, bool write_to_file, FILE *result_file)
{
    int result_decimal = binary_to_decimal(data);

    if (write_to_file)
        fprintf(result_file, "%d\n", result_decimal);
    else
        printf("%d\n", result_decimal);
}

void *result_thread(void *arguments)
{
    struct result_thread_args *args;
    args = (struct result_thread_args *)arguments;
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
                if (GET_GPIO(MISO_PIN))
                    miso_trigger = true;
                else
                    miso_trigger = false;
            } else {
                if (data_count < DATA_BIT_WIDTH) {
                    if (GET_GPIO(RESULT_PIN))
                        data[data_count] = 1;
                    else
                        data[data_count] = 0;
                }

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

void *mem_dump_thread(void *arguments)
{
    struct mem_dump_thread_args *args;
    args = (struct mem_dump_thread_args *)arguments;
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
    // int poll_ret = 0;
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
                if (GET_GPIO(MISO_PIN))
                    miso_trigger = true;
                else
                    miso_trigger = false;
            } else {
                if (data_count < num_bits) {
                    if (GET_GPIO(RESULT_PIN))
                        data[data_count] = 1;
                    else
                        data[data_count] = 0;
                }

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
    gpio = (volatile unsigned *)arguments;

    while (true) {
        clk_period = 1000000 / get_clk_freq();

        if (get_clk_exit())
            break;

        if (get_clk_enable()) {
            set_gpio_high(CLK_PIN, gpio);
            usleep((useconds_t)(clk_period / 2));
            set_gpio_low(CLK_PIN, gpio);
            usleep((useconds_t)(clk_period / 2));
        }
    }

    return NULL;
}

void *timer_ext_clk_thread(void *arguments)
{
    volatile unsigned *gpio;
    double timer_ext_clk_period = 1000000 / TIMER_EXT_CLK_FREQ_HZ;
    gpio = (volatile unsigned *)arguments;

    while (true) {
        if (get_clk_exit())
            break;

        if (get_clk_enable()) {
            set_gpio_high(TIMER_EXT_CLK_PIN, gpio);
            usleep((useconds_t)(timer_ext_clk_period / 2));
            set_gpio_low(TIMER_EXT_CLK_PIN, gpio);
            usleep((useconds_t)(timer_ext_clk_period / 2));
        }
    }

    return NULL;
}

bool send_command_to_arduino(struct command_and_args *command, FILE *result_file, char *serial_port)
{
    int fd;
    char input = '\0';

    if ((fd = serialOpen(serial_port, 9600)) < 0) {
        printf("%s, Failed to open serial connection to port %s, exiting...\n", __func__,
               serial_port);
        return false;
    }

    sleep(2); // Wait for serial connection to stabilize
    serialPuts(fd, command->full_command);

    while (true) {
        if (serialDataAvail(fd) > 0) {
            input = (char)serialGetchar(fd);

            if (input == '\n') {
                if (result_file != NULL)
                    fprintf(result_file, "%c", input);
                else
                    printf("%c", input);

                break;
            }

            if (result_file != NULL)
                fprintf(result_file, "%c", input);
            else
                printf("%c", input);
        }
    }

    serialClose(fd);
    printf("\n%s, Connection closed\n", __func__);
    return true;
}

bool send_command_to_fpga(void *arguments, struct command_and_args *command, FILE *result_file,
                          bool write_to_file)
{
    char clk_in_pin_file[MAX_STRING_SIZE];
    sprintf(clk_in_pin_file, "/sys/class/gpio/gpio%d/value", CLK_IN_PIN);
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);

    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio %d\n", __func__, CLK_IN_PIN);
        return false;
    }

    struct pollfd pfds[1];

    volatile unsigned *gpio;
    struct result_thread_args result_args;
    struct mem_dump_thread_args mem_dump_args;
    char binary_command[BINARY_COMMAND_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    pthread_t read_result_thread_id;
    pthread_t mem_dump_thread_id;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    memset(binary_command, '\0', sizeof(binary_command));
    memset(instruction, '\0', sizeof(instruction));
    gpio = (volatile unsigned *)arguments;
    result_args.gpio = gpio;
    result_args.result_file = result_file;
    result_args.write_to_file = write_to_file;
    mem_dump_args.gpio = gpio;
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    if (!create_binary_command(command, binary_command, instruction)) {
        printf("%s, Could not create binary command from command %s\n", __func__, command->command_name);
        close(clk_in_pin_fd);
        return false;
    }

    if (!get_clk_enable()) {
        printf("%s, Clock is not enabled, please enable it first\n", __func__);
        close(clk_in_pin_fd);
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

        // pthread_create(&read_result_thread_id, NULL, result_thread, (void *)&result_args);
        pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)&mem_dump_args);
        usleep(1000);
    }

    // binary_command = <opcode_in_binary> + <argument_in_binary>
    // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
    size_t length = strlen(binary_command);
    size_t idx = 0;
    int poll_ret = 0;

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

    if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
        strcmp(instruction, "READ_STATUS") == 0)
        pthread_join(read_result_thread_id, NULL);
    else if (strcmp(instruction, "DUMP_RAM") == 0 ||
             strcmp(instruction, "DUMP_EEPROM") == 0)
        pthread_join(mem_dump_thread_id, NULL);

    close(clk_in_pin_fd);
    return true;
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
