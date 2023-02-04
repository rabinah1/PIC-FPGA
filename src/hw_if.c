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

#ifndef UNIT_TEST
void set_gpio_high(int pin, volatile unsigned *gpio)
{
    GPIO_SET = 1 << pin;
}
#endif

#ifndef UNIT_TEST
void set_gpio_low(int pin, volatile unsigned *gpio)
{
    GPIO_CLR = 1 << pin;
}
#endif

int binary_to_decimal(volatile int *data)
{
    int result = 0;
    for (int idx = DATA_BIT_WIDTH - 1; idx >= 0; idx--)
        result += data[idx] * (int)pow(2, 7 - idx);

    return result;
}

void decimal_to_binary(uint32_t decimal_in, char *binary_out, int num_bits)
{
    int idx = num_bits - 1;
    bool zero_flag = false;
    memset(binary_out, '\0', sizeof(char) * (uint32_t)num_bits);
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
    char *name;
    while (idx < num_instructions) {
        name = get_slave_0_command(idx);
        if (strcmp(name, instruction) == 0) {
            memset(opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
            strcpy(opcode, get_slave_0_binary(idx));
            return true;
        }
        idx++;
    }
    printf("%s, Invalid instruction %s\n", __func__, instruction);

    return false;
}

bool create_binary_command(char *command, char *binary_command, char *instruction)
{
    char *binary_data_operand = (char *)malloc(MAX_OPERAND_SIZE);
    char *binary_data_opcode = (char *)malloc(MAX_OPCODE_SIZE);
    char *binary_data_bit_or_d = (char *)malloc(MAX_BIT_OR_D_SIZE);
    uint32_t literal_or_address = 0;
    uint32_t bit_or_d = 0;
    int num_spaces = 0;
    int idx = 0;

    while (command[idx] != '\0') {
        if (command[idx] == ' ')
            num_spaces++;
        idx++;
    }
    memset(binary_data_bit_or_d, '\0', sizeof(char) * MAX_BIT_OR_D_SIZE);
    memset(binary_data_operand, '\0', sizeof(char) * MAX_OPERAND_SIZE);
    memset(binary_data_opcode, '\0', sizeof(char) * MAX_OPCODE_SIZE);
    memset(binary_command, '\0', sizeof(char) * BINARY_COMMAND_SIZE);
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
    if (num_spaces == 2) { // bit-oriented or byte-oriented instruction
        sscanf(command, "%s %d %d", instruction, &bit_or_d, &literal_or_address);
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
    } else if (num_spaces == 1) { // literal instruction
        sscanf(command, "%s %d", instruction, &literal_or_address);
        if (!get_command_in_binary(instruction, binary_data_opcode)) {
            free(binary_data_opcode);
            free(binary_data_operand);
            free(binary_data_bit_or_d);
            return false;
        }
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    } else if (num_spaces == 0) { // Other instruction
        sscanf(command, "%s", instruction);
        if (!get_command_in_binary(instruction, binary_data_opcode)) {
            free(binary_data_opcode);
            free(binary_data_operand);
            free(binary_data_bit_or_d);
            return false;
        }
        literal_or_address = 0;
        decimal_to_binary(literal_or_address, binary_data_operand, 8);
        sprintf(binary_command, "%s%s", binary_data_opcode, binary_data_operand);
    } else {
        printf("%s, Invalid number of spaces %d in command %s\n", __func__, num_spaces, command);
        free(binary_data_opcode);
        free(binary_data_operand);
        free(binary_data_bit_or_d);
        return false;
    }
    free(binary_data_opcode);
    free(binary_data_operand);
    free(binary_data_bit_or_d);
    return true;
}

void *result_thread(void *arguments)
{
    struct result_thread_args *args = (struct result_thread_args *)arguments;
    volatile unsigned *gpio;
    volatile int data[8] = {0};
    FILE *result_file;
    bool write_to_file;
    bool miso_trigger = false;
    int data_count = 0;
    int result_decimal = 0;
    int timeout = 0;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    gpio = args->gpio;
    result_file = args->result_file;
    write_to_file = args->write_to_file;

    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio 21\n", __func__);
        return NULL;
    }
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (timeout < RESULT_TIMEOUT) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        int poll_ret = poll(pfds, 1, poll_timeout);
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
                result_decimal = binary_to_decimal(data);
                data_count = 0;
                if (write_to_file)
                    fprintf(result_file, "%d\n", result_decimal);
                else
                    printf("%d\n", result_decimal);
                result_decimal = 0;
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
    volatile unsigned *gpio;
    volatile int data[NUM_BITS_RAM] = {0};
    bool miso_trigger = false;
    int byte_data[8] = {0};
    int data_count = 0;
    int result = 0;
    int counter = 0;
    int timeout = 0;
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    gpio = (volatile unsigned *)arguments;

    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio 21\n", __func__);
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
                timeout++;
                if (data_count < NUM_BITS_RAM) {
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

int send_to_arduino(char *command, FILE *result_file, char *serial_port)
{
    int fd;
    char input = '\0';
    if ((fd = serialOpen(serial_port, 9600)) < 0) {
        printf("%s, Failed to open serial connection to port %s, exiting...\n", __func__,
               serial_port);
        return 1;
    }
    sleep(2); // Wait for serial connection to stabilize
    serialPuts(fd, command);

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
    return 0;
}

bool send_command_to_hw(char *command, void *arguments, FILE *result_file, bool write_to_file,
                        char *serial_port)
{
    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    int poll_timeout = (int)round((float)1000 / (float)get_clk_freq() * 10);
    if (clk_in_pin_fd < 0) {
        printf("%s, Error with opening file for gpio 21\n", __func__);
        return false;
    }
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;
    if (get_slave_id() == SLAVE_ID_FPGA) {
        volatile unsigned *gpio;
        struct result_thread_args args;
        char binary_command[BINARY_COMMAND_SIZE];
        char instruction[MAX_INSTRUCTION_SIZE];
        pthread_t read_result_thread_id;
        pthread_t mem_dump_thread_id;
        gpio = (volatile unsigned *)arguments;
        args.gpio = gpio;
        args.result_file = result_file;
        args.write_to_file = write_to_file;

        if (!create_binary_command(command, binary_command, instruction)) {
            printf("%s, Could not create binary command from command %s\n", __func__, command);
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
            pthread_create(&read_result_thread_id, NULL, result_thread, (void *)&args);
            usleep(1000);
        } else if (strcmp(instruction, "DUMP_MEM") == 0) {
            pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)gpio);
            usleep(1000);
        }
        // binary_command = <opcode_in_binary> + <argument_in_binary>
        // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
        size_t length = strlen(binary_command);
        size_t idx = 0;
        while (idx <= length) {
            char buff[32] = {0};
            lseek(clk_in_pin_fd, 0, SEEK_SET);
            read(clk_in_pin_fd, buff, 32);
            int poll_ret = poll(pfds, 1, poll_timeout);
            if (poll_ret == 0) {
                printf("%s, Waiting for clock edge timed out\n", __func__);
                close(clk_in_pin_fd);
                return false;
            } else if (poll_ret < 0) {
                printf("%s, Error %d happened for poll\n", __func__,
                       errno);
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
        else if (strcmp(instruction, "DUMP_MEM") == 0)
            pthread_join(mem_dump_thread_id, NULL);
        close(clk_in_pin_fd);
        return true;
    } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
        char cmd[MAX_STRING_SIZE];
        bool command_found = false;
        int num_instructions = get_num_instructions_slave_1();
        unsigned int idx = 0;
        memset(cmd, '\0', sizeof(char) * MAX_STRING_SIZE);

        while (command[idx] != '\0') {
            if (command[idx] == ' ')
                break;
            if (command[idx] == '\n') {
                command[idx] = '\0';
                break;
            }
            idx++;
        }
        strncpy(cmd, command, idx);
        for (int i = 0; i < num_instructions; i++) {
            if (strcmp(cmd, get_slave_1_command(i)) == 0) {
                command_found = true;
                break;
            }
        }
        if (command_found) {
            send_to_arduino(command, result_file, serial_port);
            close(clk_in_pin_fd);
            return true;
        } else {
            printf("%s, Command \"%s\" is not valid for slave 1\n", __func__, cmd);
            close(clk_in_pin_fd);
            return false;
        }
    } else {
        printf("%s, Invalid slave_id %d, cannot process command \"%s\"\n", __func__,
               get_slave_id(), command);
        close(clk_in_pin_fd);
        return false;
    }
    close(clk_in_pin_fd);
}
