#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/mman.h>

#define LWH2F_BRIDGE_BASE 0xFF200000
#define MMAP_LENGTH 4096
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

volatile uint16_t *op_1;
volatile uint16_t *op_2;
volatile uint16_t *ret;
const int MAX_DELAY = 1000;

int setup_memory_access(void **virtual_base)
{
    int mem_fd;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("%s, Can't open /dev/mem \n", __func__);
        return EXIT_FAILURE;
    }

    *virtual_base = mmap(NULL, MMAP_LENGTH, (PROT_READ | PROT_WRITE), MAP_SHARED, mem_fd,
                        LWH2F_BRIDGE_BASE);

    if (*virtual_base == MAP_FAILED) {
        perror("mmap");
        close(mem_fd);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main(int argc, char *argv[])
{
    int timeout = 0;
    uint16_t sum = 0;
    void *virtual_base = NULL;

    if (setup_memory_access(&virtual_base) == EXIT_FAILURE)
        return EXIT_FAILURE;

    op_1 = (uint16_t *)virtual_base;
    op_2 = (uint16_t *)(virtual_base + 0x2);
    ret = (uint16_t *)(virtual_base + 0x4);
    printf("Operand 1 is %d\n", *op_1 & 0x7FFF);
    printf("Operand 2 is %d\n", *op_2 & 0x7FFF);
    *op_1 = (uint16_t)atoi(argv[1]);
    *op_2 = (uint16_t)atoi(argv[2]);

    while (true) {
        if (timeout > MAX_DELAY) {
            printf("Could not calculate sum\n");
            break;
        }

        sum = *ret;

        if (sum & 1 << 0xF) {
            sum = sum & 0x7FFF;
            printf("Sum = %d\n", sum);
            break;
        }

        timeout++;
    }

    return EXIT_SUCCESS;
}
