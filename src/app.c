#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#ifndef UNIT_TEST
#include <wiringPi.h>
#else
#include "mocks/wiringPi.h"
#endif
#include "gpio_setup.h"
#include "functions.h"
#include "defines.h"
#include "fpga_if.h"

static const char *usage = "\
Usage: This is the control software to be run on the Raspberry Pi 4 \n\n\
\
Arguments:\n\
    serial_port (mandatory)  Serial port to which Arduino Nano is connected to\n\
    testing (optional)       This will trigger the test automation\n\
";

bool is_tb_input_valid(FILE *tb_input)
{
    char line[MAX_STRING_SIZE];
    memset(line, '\0', sizeof(char) * MAX_STRING_SIZE);
    while (fgets(line, sizeof(line), tb_input)) {
        if (line[0] == '#' || line[0] == '*')
            continue;
        line[strlen(line) - 1] = '\0';
        int ret = handle_slave_commands(line);
        if (ret == 0)
            continue;
        else if (ret == 1)
            return false;
        if (!is_command_valid(line))
            return false;
    }

    return true;
}

void run_tests(char *serial_port, volatile unsigned *gpio)
{
    printf("%s, Running tests on HW, please wait...\n", __func__);
    FILE *tb_input;
    FILE *result_file;
    char pwd[MAX_STRING_SIZE];
    char tb_input_file[3 * MAX_STRING_SIZE];
    char tb_result_file[3 * MAX_STRING_SIZE];
    char line[MAX_STRING_SIZE];
    char header_start[MAX_STRING_SIZE];
    char result_header[MAX_STRING_SIZE];
    int test_num = 0;
    memset(pwd, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(tb_input_file, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(tb_result_file, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(line, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(header_start, '\0', sizeof(char) * MAX_STRING_SIZE);
    memset(result_header, '\0', sizeof(char) * MAX_STRING_SIZE);
    getcwd(pwd, sizeof(char) * MAX_STRING_SIZE);
    sprintf(tb_input_file, "%s/test_data/real_hw_tb_input.txt", pwd);
    sprintf(tb_result_file, "%s/test_data/real_hw_tb_result.txt", pwd);
    tb_input = fopen(tb_input_file, "r");
    result_file = fopen(tb_result_file, "w");

    if (is_tb_input_valid(tb_input)) {
        rewind(tb_input);
        while (fgets(line, sizeof(line), tb_input)) {
            int ret = handle_slave_commands(line);
            if (ret == 0)
                continue;
            else if (ret == 1)
                break;
            if (line[0] == '*' || (line[0] == '#' && strstr(line, "test ") == NULL))
                continue;
            if (line[0] == '#' && strstr(line, "test ") != NULL) {
                sscanf(line, "# %s %d", header_start, &test_num);
                strcpy(result_header, "# result ");
                fprintf(result_file, "%s%d\n", result_header, test_num);
                continue;
            }
            line[strlen(line) - 1] = '\0';
            if (!process_command(line, (void *)gpio, result_file, true, serial_port))
                break;
            usleep(200000);
        }
    } else {
        printf("%s, Command %s in testbench input file is not valid, not running tests\n",
               __func__, line);
    }
    fclose(tb_input);
    fclose(result_file);
    printf("%s, Tests finished\n", __func__);
}

void run_app(char *serial_port, bool is_valid, volatile unsigned *gpio)
{
    char filename[MAX_STRING_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    printf("%s, Please enter command, or \"HELP\" for instructions\n", __func__);
    while (true) {
        char *command = malloc(MAX_STRING_SIZE);
        memset(filename, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
        fgets(command, MAX_STRING_SIZE, stdin);
        int ret = handle_slave_commands(command);
        if (ret == 0 || ret == 1)
            continue;
        if (get_slave_id() == SLAVE_ID_FPGA) {
            if (strstr(command, "READ_FILE") == NULL) {
                if (is_command_valid(command)) {
                    if (!process_command(command, (void *)gpio, NULL, false, serial_port)) {
                        free(command);
                        break;
                    }
                }
            } else {
                sscanf(command, "%s %s", instruction, filename);
                FILE *input_file = fopen(filename, "r");
                char line[MAX_STRING_SIZE];
                memset(line, '\0', sizeof(char) * MAX_STRING_SIZE);
                is_valid = true;
                while (fgets(line, sizeof(line), input_file)) {
                    line[strlen(line) - 1] = '\0'; // Remove trailing newline
                    if (!is_command_valid(line)) {
                        is_valid = false;
                        break;
                    }
                }
                if (is_valid) {
                    rewind(input_file);
                    while (fgets(line, sizeof(line), input_file)) {
                        line[strlen(line) - 1] = '\0';
                        if (!process_command(line, (void *)gpio, NULL, false, serial_port))
                            break;
                        usleep(200000);
                    }
                }
                fclose(input_file);
            }
        } else if (get_slave_id() == SLAVE_ID_ARDUINO) {
            if (is_command_valid(command)) {
                if (!process_command(command, (void *)gpio, NULL, false, serial_port)) {
                    free(command);
                    break;
                }
            }
        }
        free(command);
    }
}

int main(int argc, char *argv[])
{
    if (wiringPiSetup() < 0) {
        printf("%s, Setting up wiringPi failed, exiting...\n", __func__);
        return 1;
    }
    char serial_port[MAX_STRING_SIZE];
    bool verify_on_hw = false;

    if (argc == 3) {
        strcpy(serial_port, argv[1]);
        if (strcmp(argv[2], "testing") != 0) {
            printf("%s, Invalid argument %s, it should be 'testing'\n", argv[2], __func__);
            return 1;
        }
        printf("%s, Running in testing mode, using %s as a serial port to Arduino\n",
               __func__, serial_port);
        verify_on_hw = true;
    } else if (argc == 2) {
        strcpy(serial_port, argv[1]);
        printf("%s, Running in application mode, using %s as a serial port to Arduino\n",
               __func__, serial_port);
    } else {
        puts(usage);
        return 1;
    }
    bool is_valid = true;
    pthread_t clk_thread_id;
    pthread_t timer_ext_clk_thread_id;
    volatile unsigned *gpio = init_gpio_map();
    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);
    pthread_create(&timer_ext_clk_thread_id, NULL, timer_ext_clk_thread, (void *)gpio);

    if (verify_on_hw)
        run_tests(serial_port, gpio);
    else
        run_app(serial_port, is_valid, gpio);

    pthread_join(clk_thread_id, NULL);
    pthread_join(timer_ext_clk_thread_id, NULL);
    exit(0);
}
