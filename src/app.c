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
#include "mock_wiringPi.h"
#endif
#include "gpio_setup.h"
#include "common_data.h"
#include "defines.h"
#include "hw_if.h"
#include "process_command.h"

static const char *usage = "\
Usage: This is the control software to be run on the Raspberry Pi 4 \n\n\
\
Arguments:\n\
    -h/--help                Print this message and exit\n\
    serial_port (mandatory)  Serial port to which Arduino Nano is connected to\n\
    testing (optional)       This will trigger the test automation\n\
";

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
    int sw_ret = 0;
    bool hw_ret = false;
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

    while (fgets(line, sizeof(char) * MAX_STRING_SIZE, tb_input)) {
        if (line[0] == '*' || (line[0] == '#' && strstr(line, "test ") == NULL))
            continue;
        if (line[0] == '#' && strstr(line, "test ") != NULL) {
            sscanf(line, "# %s %d", header_start, &test_num);
            strcpy(result_header, "# result ");
            fprintf(result_file, "%s%d\n", result_header, test_num);
            continue;
        }
        line[strlen(line) - 1] = '\0';
        if (is_sw_command(line)) {
            sw_ret = process_sw_command(line, gpio);
            if (sw_ret == -1) {
                printf("%s, Exiting...\n", __func__);
                break;
            } else if (sw_ret == 1) {
                printf("%s, Failed to process command %s\n", __func__, line);
                break;
            }
        } else if (is_hw_command(line)) {
            hw_ret = process_hw_command(line, gpio, serial_port, result_file, true);
            if (hw_ret == false) {
                printf("%s, Failed to process command %s\n", __func__, line);
                break;
            }
        } else {
            line[strlen(line) - 1] = '\0';
            printf("%s, Command %s was not recognized\n", __func__, line);
            break;
        }
        usleep(200000);
    }
    fclose(tb_input);
    fclose(result_file);
    printf("%s, Tests finished\n", __func__);
}

void run_app(char *serial_port, volatile unsigned *gpio)
{
    printf("%s, Please enter command, or \"HELP\" for instructions\n", __func__);
    int sw_ret = 0;
    bool hw_ret = false;

    while (true) {
        char *command = malloc(MAX_STRING_SIZE);
        fgets(command, MAX_STRING_SIZE, stdin);
        if (is_sw_command(command)) {
            sw_ret = process_sw_command(command, gpio);
            if (sw_ret == -1) {
                printf("%s, Exiting...\n", __func__);
                free(command);
                break;
            } else if (sw_ret == 1) {
                printf("%s, Failed to process command %s\n", __func__, command);
            }
        } else if (is_hw_command(command)) {
            hw_ret = process_hw_command(command, gpio, serial_port, NULL, false);
            if (hw_ret == false)
                printf("%s, Failed to process command %s\n", __func__, command);
        } else {
            command[strlen(command) - 1] = '\0';
            printf("%s, Command %s was not recognized\n", __func__, command);
        }
        free(command);
    }
}

int parse_args(int argc, char *serial_port, char *argv[])
{
    int mode = INVALID_MODE;

    if (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0) {
        puts(usage);
    } else if (argc == 3) {
        strcpy(serial_port, argv[1]);
        if (strcmp(argv[2], "testing") != 0) {
            printf("%s, Invalid argument %s, it should be 'testing'\n", argv[2], __func__);
        } else {
            printf("%s, Running in testing mode, using %s as a serial port to Arduino\n",
                   __func__, serial_port);
            mode = TESTING_MODE;
        }
    } else if (argc == 2) {
        strcpy(serial_port, argv[1]);
        printf("%s, Running in application mode, using %s as a serial port to Arduino\n",
               __func__, serial_port);
        mode = APPLICATION_MODE;
    } else {
        puts(usage);
    }

    return mode;
}

int main(int argc, char *argv[])
{
    if (wiringPiSetup() < 0) {
        printf("%s, Setting up wiringPi failed, exiting...\n", __func__);
        return 1;
    }
    char serial_port[MAX_STRING_SIZE];
    pthread_t clk_thread_id;
    pthread_t timer_ext_clk_thread_id;
    volatile unsigned *gpio = init_gpio_map();
    if (gpio == NULL) {
        printf("%s, Failed to initialize GPIO, exiting...\n", __func__);
        return 1;
    }
    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);
    pthread_create(&timer_ext_clk_thread_id, NULL, timer_ext_clk_thread, (void *)gpio);
    int mode = parse_args(argc, serial_port, argv);
    if (mode == TESTING_MODE)
        run_tests(serial_port, gpio);
    else if (mode == APPLICATION_MODE)
        run_app(serial_port, gpio);
    else
        printf("%s, Invalid mode %d was detected, exiting\n", __func__, mode);
    set_clk_exit(true);
    pthread_join(clk_thread_id, NULL);
    pthread_join(timer_ext_clk_thread_id, NULL);

    return 0;
}
