#define BCM2835_PERI_BASE 0xFE000000  // peripheral base address
#define GPIO_BASE (BCM2835_PERI_BASE + 0x200000) // GPIO controller base address
#define BLOCK_SIZE (4*20) // only using gpio registers region
#ifndef UNIT_TEST
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#endif
#define GET_GPIO(g) (*(gpio+13)&(1<<g))
#define MAX_OPERAND_SIZE 8
#define MAX_OPCODE_SIZE 6
#define MAX_BIT_OR_D_SIZE 3
#define BINARY_COMMAND_SIZE 14
#define MAX_INSTRUCTION_SIZE 32
#define CLK_FREQ_DEFAULT 4000
#define TIMER_EXT_CLK_FREQ_HZ 1
#define CLK_PIN 5
#define CLK_IN_PIN 21
#define RESET_PIN 6
#define MISO_PIN 13
#define RESULT_PIN 16
#define MOSI_PIN 19
#define DATA_PIN 26
#define TIMER_EXT_CLK_PIN 25
#define GPIO_SET *(gpio+7)
#define GPIO_CLR *(gpio+10)
#define MAX_STRING_SIZE 256
#define RESULT_TIMEOUT 1000
#define MEM_DUMP_TIMEOUT 6000
#define DATA_BIT_WIDTH 8
#define MEM_DUMP_VALUES_PER_LINE 8
#define NUM_BYTES_RAM 127
#define NUM_BITS_RAM 1016
#define SLAVE_ID_FPGA 0
#define SLAVE_ID_ARDUINO 1
#define POLL_GPIO (POLLPRI | POLLERR)
#define INVALID_MODE -1
#define APPLICATION_MODE 0
#define TESTING_MODE 1
