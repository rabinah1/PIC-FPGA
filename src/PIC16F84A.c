#include <stdio.h>
#include <stdint.h>
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
#include "code.h"
#include "defines.h"

bool clk_enable = false;
bool clk_exit = false;
char serial_port[MAX_STRING_SIZE];
char slave_1_commands[][MAX_STRING_SIZE] = {"read_temperature"};

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
           "read_temperature\n\n");
    return;
}

void init_pins(void *arguments)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)arguments;

    int arr[] = {CLK_PIN, RESET_PIN, DATA_PIN, MOSI_PIN};
    size_t len = sizeof(arr) / sizeof(arr[0]);
    uint16_t idx = 0;
    while (idx < len) {
        INP_GPIO(arr[idx]); // Pin cannot be set as output unless first set as input
        OUT_GPIO(arr[idx]);
        idx++;
    }
    INP_GPIO(RESULT_PIN);
    INP_GPIO(MISO_PIN);
}

volatile unsigned* init_gpio_map(void)
{
    // open file for mapping
    int mem_fd;
    void *gpio_map;
    volatile unsigned *gpio;
    if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
        printf("Can't open /dev/mem \n");
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

void *result_thread(void *arguments)
{
    struct result_thread_args *args = (struct result_thread_args *)arguments;
    volatile unsigned *gpio;
    FILE *result_file;
    bool write_to_file;
    gpio = args->gpio;
    result_file = args->result_file;
    write_to_file = args->write_to_file;

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
                if (write_to_file)
                    fprintf(result_file, "%d\n", result_decimal);
                else
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

void *mem_dump_thread(void *arguments)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)arguments;

    volatile int data[NUM_BITS_RAM] = {0};
    int miso_trigger = 0;
    int byte_data[8] = {0};
    int falling_check = 0;
    int data_count = 0;
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
            if (data_count < NUM_BITS_RAM) {
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

void *clk_thread(void *arguments)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)arguments;

    double clk_period = 1000000 / CLK_FREQ_HZ;
    while(1) {
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

bool is_command_valid(char *command)
{
    int i = 0;
    int num_spaces = 0;
    int arg_1 = 0;
    int arg_2 = 0;
    char instruction[MAX_INSTRUCTION_SIZE];

    while (command[i] != '\0') {
        if (command[i] == ' ')
            num_spaces++;
        i++;
    }
    memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
    if (num_spaces == 2) { // bit- or byte-oriented instruction
        if (sscanf(command, "%s %d %d", instruction, &arg_1, &arg_2) != 3) {
            printf("Failed to parse command %s\n", command);
            return false;
        }
    } else if (num_spaces == 1) { // literal instruction
        if (sscanf(command, "%s %d", instruction, &arg_2) != 2) {
            printf("Failed to parse command %s\n", command);
            return false;
        }
    } else if (num_spaces == 0) {
        if (sscanf(command, "%s", instruction) != 1) {
            printf("Failed to parse command %s\n", command);
            return false;
        }
    } else {
        printf("Invalid number of spaces (%d) in command %s", num_spaces, command);
        return false;
    }

    if (!instruction_exists(instruction))
        return false;

    if (get_expected_num_of_arguments(instruction) != num_spaces) {
        printf("Invalid number of arguments (%d) for instruction %s\n", num_spaces, instruction);
        return false;
    }

    return true;
}

bool process_command(char *command, void *arguments, FILE *result_file, bool write_to_file)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)arguments;
    char binary_data_operand[MAX_OPERAND_SIZE];
    char binary_data_opcode[MAX_OPCODE_SIZE];
    char binary_data_bit_or_d[MAX_BIT_OR_D_SIZE];
    char binary_command[BINARY_COMMAND_SIZE];
    char instruction[MAX_INSTRUCTION_SIZE];
    pthread_t read_result_thread_id;
    pthread_t mem_dump_thread_id;
    int falling_check = 0;
    struct result_thread_args args;
    args.gpio = gpio;
    args.result_file = result_file;
    args.write_to_file = write_to_file;

    if (!create_binary_command(command, binary_command, instruction, binary_data_operand,
                               binary_data_opcode, binary_data_bit_or_d)) {
        printf("Binary command creation from command %s failed\n", command);
        return false;
    }
    if (strcmp(instruction, "ENABLE_CLOCK") == 0) {
        clk_enable = true;
        return true;
    } else if (strcmp(instruction, "DISABLE_CLOCK") == 0) {
        clk_enable = false;
        return true;
    } else if (strcmp(instruction, "ENABLE_RESET") == 0) {
        GPIO_SET = 1<<RESET_PIN;
        return true;
    } else if (strcmp(instruction, "DISABLE_RESET") == 0) {
        GPIO_CLR = 1<<RESET_PIN;
        return true;
    } else if (strcmp(instruction, "EXIT") == 0) {
        GPIO_CLR = 1<<RESET_PIN;
        clk_exit = true;
        return false;
    } else if (strcmp(instruction, "HELP") == 0) {
        print_help();
        return true;
    } else if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
               strcmp(instruction, "READ_STATUS") == 0) {
        if (!clk_enable) {
            printf("Clock is not enabled, please enable it first\n");
            return true;
        }
        pthread_create(&read_result_thread_id, NULL, result_thread, (void *)&args);
        usleep(1000);
    } else if (strcmp(instruction, "DUMP_MEM") == 0) {
        if (!clk_enable) {
            printf("Clock is not enabled, please enable it first\n");
            return true;
        }
        pthread_create(&mem_dump_thread_id, NULL, mem_dump_thread, (void *)gpio);
        usleep(1000);
    }
    if (!clk_enable) {
        printf("Clock is not enabled, please enable it first\n");
        return true;
    }
    // binary_command = <opcode_in_binary> + <argument_in_binary>
    // This data is sent to FPGA one bit at a time, starting from the first (idx = 0) bit.
    uint32_t length = strlen(binary_command);
    uint32_t idx = 0;
    while (idx <= length) {
        if (!(GET_GPIO(CLK_PIN)) && falling_check == 0) { // falling edge
            falling_check = 1;
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
        } else if (GET_GPIO(CLK_PIN) && falling_check == 1) { // rising edge
            falling_check = 0;
        }
    }
    if (strcmp(instruction, "READ_WREG") == 0 || strcmp(instruction, "READ_ADDRESS") == 0 ||
        strcmp(instruction, "READ_STATUS") == 0)
        pthread_join(read_result_thread_id, NULL);
    else if (strcmp(instruction, "DUMP_MEM") == 0)
        pthread_join(mem_dump_thread_id, NULL);
    return true;
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
    bool verify_on_hw = false;

    if (argc == 2) {
        printf("Running in application mode, using %s as a serial port to Arduino\n", argv[1]);
    } else if (argc == 3 && strcmp(argv[2], "testing") == 0) {
        printf("Running in testing mode, using %s as a serial port to Arduino\n", argv[1]);
        verify_on_hw = true;
    } else {
        printf("Invalid arguments\n");
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

    if (verify_on_hw) {
        printf("Running tests on HW, please wait...\n");
        char pwd[MAX_STRING_SIZE];
        char script_relative_path[MAX_STRING_SIZE];
        char tb_input_file[MAX_STRING_SIZE];
        char tb_result_file[MAX_STRING_SIZE];
        char line[MAX_STRING_SIZE];
        char header_start[MAX_STRING_SIZE];
        char result_header[MAX_STRING_SIZE];
        char *last_slash;
        int test_num = 0;
        memset(pwd, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(script_relative_path, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(tb_input_file, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(tb_result_file, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(line, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(header_start, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(result_header, '\0', sizeof(char) * MAX_STRING_SIZE);
        strcpy(script_relative_path, argv[0]);
        last_slash = strrchr(script_relative_path, '/');
        script_relative_path[last_slash - script_relative_path] = '\0';
        getcwd(pwd, sizeof(char) * MAX_STRING_SIZE);
        strcpy(tb_input_file, pwd);
        strcat(tb_input_file, "/");
        strcat(tb_input_file, script_relative_path);
        strcat(tb_input_file, "/");
        strcat(tb_input_file, "real_hw_tb_input.txt");
        strcpy(tb_result_file, pwd);
        strcat(tb_result_file, "/");
        strcat(tb_result_file, script_relative_path);
        strcat(tb_result_file, "/");
        strcat(tb_result_file, "real_hw_tb_result.txt");
        FILE *tb_input = fopen(tb_input_file, "r");
        FILE *result_file = fopen(tb_result_file, "w");
        is_valid = true;
        while (fgets(line, sizeof(line), tb_input)) {
            if (line[0] == '#' || line[0] == '*')
                continue;
            line[strlen(line) - 1] = '\0';
            if (!is_command_valid(line)) {
                is_valid = false;
                break;
            }
        }
        if (is_valid) {
            rewind(tb_input);
            while (fgets(line, sizeof(line), tb_input)) {
                if (line[0] == '*' || (line[0] == '#' && strstr(line, "test ") == NULL))
                    continue;
                if (line[0] == '#' && strstr(line, "test ") != NULL) {
                    sscanf(line, "# %s %d", header_start, &test_num);
                    strcpy(result_header, "# result ");
                    fprintf(result_file, "%s%d\n", result_header, test_num);
                    continue;
                }
                line[strlen(line) - 1] = '\0';
                if (!process_command(line, (void *)gpio, result_file, true))
                    break;
                usleep(100000);
            }
        }
        fclose(tb_input);
        fclose(result_file);
        printf("Tests finished\n");
        return 0;
    }

    printf("Please enter command, or \"HELP\" for instructions\n");
    while(1) {
        memset(filename, '\0', sizeof(char) * MAX_STRING_SIZE);
        memset(instruction, '\0', sizeof(char) * MAX_INSTRUCTION_SIZE);
        char *command = malloc(MAX_STRING_SIZE);
        fgets(command, MAX_STRING_SIZE, stdin);
        if (strstr(command, "SELECT_SLAVE") != NULL) {
            if (is_command_valid(command))
                sscanf(command, "%s %d", instruction, &slave_sel);

        } else if (strstr(command, "SHOW_SLAVE") != NULL) {
            if (is_command_valid(command))
                printf("Slave %d\n", slave_sel);

        } else if (slave_sel == 0) { // Slave is FPGA
            if (strstr(command, "READ_FILE") == NULL) { // READ_FILE not found
                if (is_command_valid(command)) {
                    if (!process_command(command, (void *)gpio, NULL, false)) {
                        free(command);
                        break;
                    }
                }
            } else { // READ_FILE found
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
                        if (!process_command(line, (void *)gpio, NULL, false))
                            break;
                        usleep(100000);
                    }
                }
                fclose(input_file);
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
