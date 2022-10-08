#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/poll.h>
#include <errno.h>
#include "defines.h"
#include "functions.h"
#include "arduino.h"

bool clk_enable = false;
bool clk_exit = false;
struct result_thread_args {
    volatile unsigned *gpio;
    FILE *result_file;
    bool write_to_file;
};

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
           "SHOW_SLAVE\n"
           "SELECT_SLAVE\n"
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
           "read_temperature\n"
           "echo <message>\n\n");
    return;
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
    gpio = args->gpio;
    result_file = args->result_file;
    write_to_file = args->write_to_file;

    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    if (clk_in_pin_fd < 0) {
        printf("Error with opening file for gpio 21\n");
        return NULL;
    }
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (timeout < RESULT_TIMEOUT) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        int poll_ret = poll(pfds, 1, 2);
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
    gpio = (volatile unsigned *)arguments;

    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    if (clk_in_pin_fd < 0) {
        printf("Error with opening file for gpio 21\n");
        return NULL;
    }
    pfds[0].fd = clk_in_pin_fd;
    pfds[0].events = POLL_GPIO;

    while (timeout < MEM_DUMP_TIMEOUT) {
        char buff[32] = {0};
        lseek(clk_in_pin_fd, 0, SEEK_SET);
        read(clk_in_pin_fd, buff, 32);
        int poll_ret = poll(pfds, 1, 2);
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
    double clk_period = 1000000 / CLK_FREQ_HZ;
    gpio = (volatile unsigned *)arguments;

    while (true) {
        if (clk_exit)
            break;
        if (clk_enable) {
            GPIO_SET = 1 << CLK_PIN;
            usleep(clk_period / 2);
            GPIO_CLR = 1 << CLK_PIN;
            usleep(clk_period / 2);
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
        if (clk_exit)
            break;
        if (clk_enable) {
            GPIO_SET = 1 << TIMER_EXT_CLK_PIN;
            usleep(timer_ext_clk_period / 2);
            GPIO_CLR = 1 << TIMER_EXT_CLK_PIN;
            usleep(timer_ext_clk_period / 2);
        }
    }

    return NULL;
}

bool process_command(char *command, void *arguments, FILE *result_file, bool write_to_file, char *serial_port)
{
    struct pollfd pfds[1];
    char clk_in_pin_file[MAX_STRING_SIZE] = "/sys/class/gpio/gpio21/value";
    int clk_in_pin_fd = open(clk_in_pin_file, O_RDONLY);
    if (clk_in_pin_fd < 0) {
        printf("Error with opening file for gpio 21\n");
        return 1;
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
            return true;
        }
        if (strcmp(instruction, "ENABLE_CLOCK") == 0) {
            clk_enable = true;
            close(clk_in_pin_fd);
            return true;
        } else if (strcmp(instruction, "DISABLE_CLOCK") == 0) {
            clk_enable = false;
            close(clk_in_pin_fd);
            return true;
        } else if (strcmp(instruction, "ENABLE_RESET") == 0) {
            GPIO_SET = 1<<RESET_PIN;
            close(clk_in_pin_fd);
            return true;
        } else if (strcmp(instruction, "DISABLE_RESET") == 0) {
            GPIO_CLR = 1<<RESET_PIN;
            close(clk_in_pin_fd);
            return true;
        } else if (strcmp(instruction, "EXIT") == 0) {
            GPIO_CLR = 1<<RESET_PIN;
            close(clk_in_pin_fd);
            clk_exit = true;
            return false;
        } else if (strcmp(instruction, "HELP") == 0) {
            print_help();
            close(clk_in_pin_fd);
            return true;
        } else if (strcmp(instruction, "READ_WREG") == 0 ||
                   strcmp(instruction, "READ_ADDRESS") == 0 ||
                   strcmp(instruction, "READ_STATUS") == 0) {
            if (!clk_enable) {
                printf("%s, Clock is not enabled, please enable it first\n", __func__);
                close(clk_in_pin_fd);
                return true;
            }
            pthread_create(&read_result_thread_id, NULL, result_thread, (void *)&args);
            usleep(1000);
        } else if (strcmp(instruction, "DUMP_MEM") == 0) {
            if (!clk_enable) {
                printf("%s, Clock is not enabled, please enable it first\n", __func__);
                close(clk_in_pin_fd);
                return true;
            }
            pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)gpio);
            usleep(1000);
        }
        if (!clk_enable) {
            printf("%s, Clock is not enabled, please enable it first\n", __func__);
            close(clk_in_pin_fd);
            return true;
        }
        // binary_command = <opcode_in_binary> + <argument_in_binary>
        // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
        uint32_t length = strlen(binary_command);
        uint32_t idx = 0;
        while (idx <= length) {
            char buff[32] = {0};
            lseek(clk_in_pin_fd, 0, SEEK_SET);
            read(clk_in_pin_fd, buff, 32);
            int poll_ret = poll(pfds, 1, 2);
            if (poll_ret == 0) {
                printf("%s, Waiting for clock edge timed out\n", __func__);
                close(clk_in_pin_fd);
                return NULL;
            } else if (poll_ret < 0) {
                printf("%s, Error %d happened for poll in functio process_command\n", __func__,
                       errno);
                close(clk_in_pin_fd);
                return NULL;
            } else if (poll_ret > 0 && !(GET_GPIO(CLK_PIN)) && (pfds[0].revents & POLL_GPIO)) {
                // falling edge
                if (idx < length) {
                    if (idx == 0)
                        GPIO_SET = 1 << MOSI_PIN;
                    if (binary_command[idx] == '0')
                        GPIO_CLR = 1 << DATA_PIN;
                    else
                        GPIO_SET = 1 << DATA_PIN;
                } else {
                    GPIO_CLR = 1 << MOSI_PIN;
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
            return true;
        }
    } else {
        printf("%s, Invalid slave_id %d, cannot process command \"%s\"\n", __func__,
               get_slave_id(), command);
        close(clk_in_pin_fd);
        return true;
    }
    close(clk_in_pin_fd);
}
