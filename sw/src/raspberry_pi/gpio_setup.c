#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#ifndef UNIT_TEST
#include <fcntl.h>
#include <sys/mman.h>
#else
#include "mock_fcntl.h"
#include "mock_mman.h"
#include "mock_gpio.h"
#endif
#include "defines.h"
#include "gpio_setup.h"
#include "logger/src/log.h"

#define BLOCK_SIZE (4*20) // only using gpio registers region
#define BCM2835_PERI_BASE 0xFE000000  // peripheral base address
#define GPIO_BASE (BCM2835_PERI_BASE + 0x200000) // GPIO controller base address
#ifndef UNIT_TEST
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#endif

void init_pins(void *gpio_void)
{
    volatile unsigned *gpio = (volatile unsigned *)gpio_void;
    (void) gpio;
    int out_pins[] = {CLK_PIN, TIMER_EXT_CLK_PIN, RESET_PIN, DATA_PIN, MOSI_PIN};
    int in_pins[] = {RESULT_PIN, MISO_PIN, CLK_IN_PIN};
    size_t num_out_pins = sizeof(out_pins) / sizeof(out_pins[0]);
    size_t num_in_pins = sizeof(in_pins) / sizeof(in_pins[0]);
    size_t idx = 0;

    for (idx = 0; idx < num_out_pins; idx++) {
        INP_GPIO(out_pins[idx]); // Pin cannot be set as output unless first set as input
        OUT_GPIO(out_pins[idx]);
    }

    for (idx = 0; idx < num_in_pins; idx++)
        INP_GPIO(in_pins[idx]);
}

volatile unsigned *init_gpio_map(void)
{
    // open file for mapping
    int mem_fd;
    void *gpio_map;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        log_error("Can't open /dev/mem: %s", strerror(errno));
        return NULL;
    }

    gpio_map = mmap(NULL,                   // Any adddress in our space will do
                    BLOCK_SIZE,             // Map length
                    PROT_READ | PROT_WRITE, // Enable reading & writting to mapped memory
                    MAP_SHARED,             // Shared with other processes
                    mem_fd,                 // File to map
                    GPIO_BASE);             // Offset to GPIO peripheral

    if (close(mem_fd) < 0)
        log_error("Can't close /dev/mem: %s", strerror(errno));

    if (gpio_map == MAP_FAILED) {
        log_error("mmap error %ld", (long unsigned int)gpio_map);
        return NULL;
    }

    return (volatile unsigned *)gpio_map;
}
