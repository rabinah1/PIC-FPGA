#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
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

void init_pins(void *gpio_void)
{
    volatile unsigned *gpio = (volatile unsigned *)gpio_void;
    (void) gpio;
    int out_pins[] = {CLK_PIN, TIMER_EXT_CLK_PIN, RESET_PIN, DATA_PIN, MOSI_PIN};
    size_t num_out_pins = sizeof(out_pins) / sizeof(out_pins[0]);
    uint16_t idx = 0;

    while (idx < num_out_pins) {
        INP_GPIO(out_pins[idx]); // Pin cannot be set as output unless first set as input
        OUT_GPIO(out_pins[idx]);
        idx++;
    }

    INP_GPIO(RESULT_PIN);
    INP_GPIO(MISO_PIN);
    INP_GPIO(CLK_IN_PIN);
}

volatile unsigned *init_gpio_map(void)
{
    // open file for mapping
    int mem_fd;
    void *gpio_map;
    volatile unsigned *gpio;

    if ((mem_fd = open("/dev/mem", O_RDWR | O_SYNC)) < 0) {
        printf("%s, Can't open /dev/mem \n", __func__);
        return NULL;
    }

    gpio_map = mmap(NULL,                   // Any adddress in our space will do
                    BLOCK_SIZE,             // Map length
                    PROT_READ | PROT_WRITE, // Enable reading & writting to mapped memory
                    MAP_SHARED,             // Shared with other processes
                    mem_fd,                 // File to map
                    GPIO_BASE);             // Offset to GPIO peripheral
    close(mem_fd);

    if (gpio_map == MAP_FAILED) {
        printf("%s, mmap error %ld\n", __func__, (long unsigned int)gpio_map);
        return NULL;
    }

    gpio = (volatile unsigned *)gpio_map;
    return gpio;
}
