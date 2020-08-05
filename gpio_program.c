#define BCM2835_PERI_BASE        0x3F000000			// peripheral base address
#define GPIO_BASE		(BCM2835_PERI_BASE + 0x200000) 	// GPIO controller base address

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <math.h>

#define BLOCK_SIZE (4*20)	// only using gpio registers region
#define CLK_PIN 5
#define CLK_FREQ_HZ 600
#define RESET_PIN 6
#define RESULT_PIN 16
#define COMMAND_PIN 26
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define GET_GPIO(g) (*(gpio+13)&(1<<g))

#define MAX_COMMAND_SIZE 256

int clk_enable = 0;
int clk_exit = 0;
int reception_stop = 0;

struct mapping {
    char *command;
    char *binary;
};

struct mapping dict[] = {
			 "ADDWF", "000111",
			 "ANDWF", "000101",
			 "CLRW", "000001",
			 "COMF", "001001",
			 "DECF", "000011",
			 "DECFSZ", "001011",
			 "INCF", "001010",
			 "INCFSZ", "001111",
			 "IORWF", "000100",
			 "MOVF", "001000",
			 "SUBWF", "000010",
			 "XORWF", "000110",
			 "ADDLW", "111110",
			 "ANDLW", "111001",
			 "SUBLW", "111101",
			 "NOP", "000000"
};

struct enable_struct
{
    volatile unsigned *gpio;
    int enable;
};

struct data_struct
{
    volatile unsigned *gpio;
    char bit;
};

void init_pins(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    int arr[] = {CLK_PIN, RESET_PIN, COMMAND_PIN};
    size_t len = sizeof(arr)/sizeof(arr[0]);
    int idx = 0;
    while (idx < len) {
	INP_GPIO(arr[idx]);
	OUT_GPIO(arr[idx]);
	idx = idx + 1;
    }
    INP_GPIO(RESULT_PIN);
}

void get_command(char *key, char *value)
{
    int i = 0;
    char *name = dict[i].command;
    while (name) {
	if (strcmp(name, key) == 0) {
	    memset(value, '\0', sizeof(value));
	    strcpy(value, dict[i].binary);
	    return;
	}
	i++;
	name = dict[i].command;
    }
    return;
}

void convert_to_binary(int decimal_in, char *binary_out)
{
    int idx = 7;
    memset(binary_out, '\0', sizeof(binary_out));
    while (idx >= 0) {
	if ((int)pow(2, idx) > decimal_in) {
	    binary_out[7 - idx] = '0';
	}

	else {
	    binary_out[7 - idx] = '1';
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
		    NULL,             	// Any adddress in our space will do
		    BLOCK_SIZE,      	// Map length
		    PROT_READ|PROT_WRITE,	// Enable reading & writting to mapped memory
		    MAP_SHARED,       	// Shared with other processes
		    mem_fd,          	// File to map
		    GPIO_BASE        	// Offset to GPIO peripheral
		    );
    close(mem_fd);
  
    if (gpio_map == MAP_FAILED) {
	printf("mmap error %d\n", (int)gpio_map);
	exit(-1);
    }
    gpio = (volatile unsigned *)gpio_map;
    return gpio;
}

void *read_command(void *vargp)
{
    volatile unsigned *gpio;
    char bit;
    struct data_struct *my_data = (struct data_struct *)vargp;
    gpio = my_data->gpio;
    bit = my_data->bit;

    if (bit == '0')
	GPIO_CLR = 1<<COMMAND_PIN;

    else if (bit == '1')
	GPIO_SET = 1<<COMMAND_PIN;

    return NULL;
}

void *result_thread(void *vargp)
{
    volatile unsigned *gpio;
    gpio = (volatile unsigned *)vargp;

    volatile int code_word[6] = {0, 0, 0, 0, 0, 0};
    int code_word_correct[6] = {0, 0, 0, 1, 0, 1};
    volatile int data[8] = {0, 0, 0, 0, 0, 0, 0, 0};
    int falling_check = 0;
    int cw_found = 0;
    int data_count = 0;
    int result_decimal = 0;

    while (1) {
	if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && cw_found == 0) { // falling edge, code word not found
	    falling_check = 1;
	    for (int idx = 0; idx < 5; idx++)
		code_word[idx] = code_word[idx+1];
	    if (GET_GPIO(RESULT_PIN)) {
		code_word[5] = 1;
	    }
	    else {
		code_word[5] = 0;
	    }

	    cw_found = 1;
	    for (int idx = 0; idx < 6; idx++) {
		if (code_word[idx] != code_word_correct[idx]) {
		    cw_found = 0;
		    break;
		}
	    }
	}

	else if (!(GET_GPIO(CLK_PIN)) && falling_check == 0 && cw_found == 1) { // falling edge, code word found
	    if (data_count < 8) {
		falling_check = 1;
		for (int idx = 0; idx < 7; idx++)
		    data[idx] = data[idx+1];
		if (GET_GPIO(RESULT_PIN))
		    data[7] = 1;
		else
		    data[7] = 0;
	    }
	    data_count++;
	}

	else if (falling_check == 1 && GET_GPIO(CLK_PIN)) { // rising edge
	    falling_check = 0;
	    if (data_count == 8) {
		for (int idx = 7; idx >= 0; idx--)
		    result_decimal = result_decimal + data[idx] * (int)pow(2, 7-idx);
		cw_found = 0;
		data_count = 0;
		printf("The result is: %d\n", result_decimal);
		result_decimal = 0;
	    }
	}

	if (reception_stop == 1) // force exit
	    break;
    }
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

void *reset_thread(void *vargp)
{
    volatile unsigned *gpio;
    int reset_enable;
    struct enable_struct *my_struct = (struct enable_struct *)vargp;
    reset_enable = my_struct->enable;
    gpio = my_struct->gpio;

    if (reset_enable == 1)
	GPIO_SET = 1<<RESET_PIN;
    if (reset_enable == 0)
	GPIO_CLR = 1<<RESET_PIN;
    return NULL;
}

int main(void)
{
    char binary_data_literal[99];
    char binary_data_opcode[99];
    char binary_command[99];
    char instruction[99];
    char literal_keyword[99] = "00000110";
    char temp[99];
    int literal = 0;
    pthread_t clk_thread_id;
    pthread_t reset_thread_id;
    pthread_t read_result_thread_id;
    pthread_t command_thread_id;
    int operation;
    int falling_check = 0;
    volatile unsigned *gpio = init_gpio_map();
    struct enable_struct *ena;
    struct data_struct *data;
    ena = malloc(sizeof(struct enable_struct));
    data = malloc(sizeof(struct data_struct));
    ena->gpio = gpio;
    ena->enable = 0;
    data->gpio = gpio;
    data->bit = '0';
    init_pins((void *)gpio);
    pthread_create(&clk_thread_id, NULL, clk_thread, (void *)gpio);
    pthread_create(&read_result_thread_id, NULL, result_thread, (void *)gpio);

    while(1) {
	printf("What do you want to do?\n");
	printf("1) Enable clock\n");
	printf("2) Disable clock\n");
	printf("3) Enable reset\n");
	printf("4) Disable reset\n");
	printf("5) Give instruction\n");
	printf("6) Exit program\n");
	scanf("%d", &operation);
	getchar();
      
	if (operation == 1)
	    clk_enable = 1;

	else if (operation == 2)
	    clk_enable = 0;

	else if (operation == 3) {
	    ena->enable = 1;
	    pthread_create(&reset_thread_id, NULL, reset_thread, (void *)ena);
	    pthread_join(reset_thread_id, NULL);
	}

	else if (operation == 4) {
	    ena->enable = 0;
	    pthread_create(&reset_thread_id, NULL, reset_thread, (void *)ena);
	    pthread_join(reset_thread_id, NULL);
	}

	else if (operation == 5) {
	    // convert the instruction to 14 bits, and send them to FPGA.
	    char *command = malloc(MAX_COMMAND_SIZE);
	    fgets(command, MAX_COMMAND_SIZE, stdin);
	    sscanf(command, "%s %d", instruction, &literal);
	    convert_to_binary(literal, binary_data_literal);
	    get_command(instruction, binary_data_opcode);

	    memset(temp, '\0', sizeof(temp));
	    strcpy(temp, literal_keyword);
	    strcat(temp, binary_data_opcode);
	    strcat(temp, binary_data_literal);
	    memset(binary_command, '\0', sizeof(binary_command));
	    strcpy(binary_command, temp);

	    int length = strlen(binary_command);
	    int idx = 0;
	    while (idx < length) {
		if (!(GET_GPIO(CLK_PIN)) && falling_check == 0) {
		    falling_check = 1;
		    data->bit = binary_command[idx];
		    pthread_create(&command_thread_id, NULL, read_command, (void *)data);
		    pthread_join(command_thread_id, NULL);
		    idx++;
		}
		else if (falling_check == 1 && GET_GPIO(CLK_PIN))
		    falling_check = 0;
	    }
	    free(command);
	}

	else if (operation == 6) {
	    ena->enable = 0;
	    pthread_create(&reset_thread_id, NULL, reset_thread, (void *)ena);
	    pthread_join(reset_thread_id, NULL);
	    clk_exit = 1;
	    reception_stop = 1;
	    free(ena);
	    free(data);
	    break;
	}
    }
    pthread_join(clk_thread_id, NULL);
    pthread_join(read_result_thread_id, NULL);
    exit(0);
}
