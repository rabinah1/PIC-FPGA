#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <argp.h>
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
#include "logger/src/log.h"

#define TESTING_MODE 1
#define APPLICATION_MODE 0
#define INVALID_MODE -1

struct arguments {
    int run_tests;
    char serial_port[MAX_STRING_SIZE];
};

static bool is_comment(char *line)
{
    if (line[0] == '*' || (line[0] == '#' && strstr(line, "test ") == NULL))
        return true;

    return false;
}

static bool add_result_header(char *line, FILE *result_file)
{
    int test_num = 0;
    char header_start[MAX_STRING_SIZE];
    char result_header[MAX_STRING_SIZE];
    memset(header_start, '\0', sizeof(header_start));
    memset(result_header, '\0', sizeof(result_header));

    if (line[0] == '#' && strstr(line, "test ") != NULL) {
        sscanf(line, "# %s %d", header_start, &test_num);
        strcpy(result_header, "# result ");
        fprintf(result_file, "%s%d\n", result_header, test_num);
        return true;
    }

    return false;
}

static bool check_file_open(FILE *file, const char *filename)
{
    if (!file) {
        log_error("Failed to open file %s", filename);
        return false;
    }

    return true;
}

static bool process_command(struct command_and_args *cmd, volatile unsigned *gpio, char *cmd_str,
                            char *serial_port, FILE *result_file, bool is_app)
{
    int sw_ret = SW_SUCCESS;

    if (is_expected_command_type(cmd, "sw")) {
        sw_ret = process_sw_command(cmd, gpio);

        if (sw_ret == SW_EXIT) {
            log_info("Exiting...");
            return false;
        } else if (sw_ret == SW_FAILED) {
            log_error("Failed to process command %s", cmd_str);
            return is_app;
        }
    } else if (is_expected_command_type(cmd, "hw")) {
        if (!process_hw_command(cmd, gpio, serial_port, result_file, !is_app)) {
            log_error("Failed to process command %s", cmd_str);
            return is_app;
        }
    } else {
        cmd_str[strlen(cmd_str) - 1] = '\0';
        log_error("Command %s was not recognized", cmd_str);
        return is_app;
    }

    return true;
}

static bool process_one_line_from_tb_file(char *line, FILE *result_file, volatile unsigned *gpio,
        char *serial_port)
{
    struct command_and_args cmd;
    memset(&cmd, 0, sizeof(struct command_and_args));

    if (is_comment(line) || add_result_header(line, result_file))
        return true;

    line[strlen(line) - 1] = '\0';

    if (!verify_command_syntax(line, &cmd)) {
        log_error("Invalid command %s", line);
        return false;
    }

    if (!process_command(&cmd, gpio, line, serial_port, result_file, false))
        return false;

    usleep(PROCESS_COMMAND_DELAY_USEC);
    return true;
}

static void run_tests(char *serial_port, volatile unsigned *gpio)
{
    log_info("Running tests on HW, please wait...");
    char pwd[MAX_STRING_SIZE];
    char tb_input_file[3 * MAX_STRING_SIZE];
    char tb_result_file[3 * MAX_STRING_SIZE];
    char line[MAX_STRING_SIZE];
    memset(pwd, '\0', sizeof(pwd));
    memset(tb_input_file, '\0', sizeof(tb_input_file));
    memset(tb_result_file, '\0', sizeof(tb_result_file));
    memset(line, '\0', sizeof(line));
    getcwd(pwd, sizeof(pwd));
    sprintf(tb_input_file, "%s/test_data/data/real_hw_tb_input.txt", pwd);
    sprintf(tb_result_file, "%s/test_data/data/real_hw_tb_result.txt", pwd);
    FILE *tb_input = fopen(tb_input_file, "r");

    if (!check_file_open(tb_input, tb_input_file))
        return;

    FILE *result_file = fopen(tb_result_file, "w");

    if (!check_file_open(result_file, tb_result_file)) {
        fclose(tb_input);
        return;
    }

    while (fgets(line, sizeof(line), tb_input)) {
        if (!process_one_line_from_tb_file(line, result_file, gpio, serial_port))
            break;
    }

    fclose(tb_input);
    fclose(result_file);
    log_info("Tests finished");
}

static void run_app(char *serial_port, volatile unsigned *gpio)
{
    log_info("Please enter command, or \"HELP\" for instructions");
    char command[MAX_STRING_SIZE];
    struct command_and_args cmd;

    while (true) {
        memset(&cmd, 0, sizeof(struct command_and_args));
        memset(command, '\0', sizeof(command));
        fgets(command, MAX_STRING_SIZE, stdin);
        command[strcspn(command, "\n")] = '\0';

        if (!verify_command_syntax(command, &cmd)) {
            log_warn("Invalid command %s", command);
            continue;
        }

        if (!process_command(&cmd, gpio, cmd.command_name, serial_port, NULL, true))
            break;
    }
}

static int parse_opt(int key, char *arg, struct argp_state *state)
{
    struct arguments *arguments = (struct arguments *)state->input;

    switch (key) {
    case 777: {
        arguments->run_tests = 1;
        break;
    }

    case 888: {
        strcpy(arguments->serial_port, arg);
        break;
    }
    }

    return 0;
}

static int parse_args(int argc, char *serial_port, char *argv[])
{
    struct arguments arguments;
    int mode = INVALID_MODE;
    arguments.run_tests = 0;
    strcpy(arguments.serial_port, "/dev/ttyUSB0");
    memset(serial_port, '\0', MAX_STRING_SIZE);
    struct argp_option options[] = {
        {"run_tests", 777, 0, 0, "Run tests"},
        {
            "serial_port", 888, "PORT", 0, "Set serial port towards Arduino, "
            "default value is /dev/ttyUSB0"
        },
        {0}
    };
    struct argp argp = {options, parse_opt};
    argp_parse(&argp, argc, argv, 0, 0, &arguments);
    strcpy(serial_port, arguments.serial_port);
    mode = arguments.run_tests ? TESTING_MODE : APPLICATION_MODE;
    return mode;
}

static bool check_setup_succeeded(volatile unsigned *gpio)
{
    if (wiringPiSetup() < 0) {
        log_error("Setting up wiringPi failed, exiting...");
        return false;
    }

    if (!gpio) {
        log_error("Failed to initialize GPIO, exiting...");
        return false;
    }

    return true;
}

int main(int argc, char *argv[])
{
    char serial_port[MAX_STRING_SIZE];
    pthread_t clk_thread_id;
    pthread_t timer_ext_clk_thread_id;
    volatile unsigned *gpio = init_gpio_map();

    if (!check_setup_succeeded(gpio))
        return 1;

    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);
    pthread_create(&timer_ext_clk_thread_id, NULL, timer_ext_clk_thread, (void *)gpio);
    int mode = parse_args(argc, serial_port, argv);

    if (mode == TESTING_MODE)
        run_tests(serial_port, gpio);
    else if (mode == APPLICATION_MODE)
        run_app(serial_port, gpio);
    else
        log_error("Invalid mode %d was detected, exiting...", mode);

    set_clk_exit(true);
    pthread_join(clk_thread_id, NULL);
    pthread_join(timer_ext_clk_thread_id, NULL);
    return 0;
}
