#include <string.h>
#include <stdint.h>
extern "C"
{
#include "defines.h"
#include "gpio_setup.h"
}
#include "../cpputest/include/CppUTest/TestHarness.h"
#include "../cpputest/include/CppUTestExt/MockSupport.h"


#define O_RDWR 1
#define O_SYNC 2
#define PROT_READ 1
#define PROT_WRITE 3
#define MAP_SHARED 1
#define MAP_FAILED (void *)-1

TEST_GROUP(test_gpio_setup_group)
{
    void teardown()
    {
        mock().clear();
    }
};

TEST(test_gpio_setup_group, test_init_gpio_map_open_failed)
{
    mock().expectOneCall("open").withParameter("file", "/dev/mem").
        withParameter("opts", O_RDWR|O_SYNC).andReturnValue(-1);
    volatile unsigned *ret = init_gpio_map();
    mock().checkExpectations();
    POINTERS_EQUAL(NULL, ret);
}

TEST(test_gpio_setup_group, test_init_gpio_map_mmap_failed)
{
    mock().expectOneCall("open").withParameter("file", "/dev/mem").
        withParameter("opts", O_RDWR|O_SYNC).andReturnValue(0);
    mock().expectOneCall("mmap").withParameter("addr", (void *)0).withParameter("size", BLOCK_SIZE).
        withParameter("opts", PROT_READ|PROT_WRITE).withParameter("shared", MAP_SHARED).
        withParameter("fd", 0).withParameter("base", GPIO_BASE).andReturnValue((void *)MAP_FAILED);
    volatile unsigned *ret = init_gpio_map();
    mock().checkExpectations();
    POINTERS_EQUAL(NULL, ret);
}

TEST(test_gpio_setup_group, test_init_gpio_map_success)
{
    mock().expectOneCall("open").withParameter("file", "/dev/mem").
        withParameter("opts", O_RDWR|O_SYNC).andReturnValue(0);
    mock().expectOneCall("mmap").withParameter("addr", (void *)0).withParameter("size", BLOCK_SIZE).
        withParameter("opts", PROT_READ|PROT_WRITE).withParameter("shared", MAP_SHARED).
        withParameter("fd", 0).withParameter("base", GPIO_BASE).andReturnValue((void *)1);
    volatile unsigned *ret = init_gpio_map();
    mock().checkExpectations();
    POINTERS_EQUAL(1, ret);
}

TEST(test_gpio_setup_group, test_init_pins)
{
    int expected_pins[] = {CLK_PIN, TIMER_EXT_CLK_PIN, RESET_PIN, DATA_PIN, MOSI_PIN};
    size_t num_out_pins = sizeof(expected_pins) / sizeof(expected_pins[0]);
    uint16_t idx = 0;
    while (idx < num_out_pins) {
        mock().expectOneCall("INP_GPIO").withParameter("pin", expected_pins[idx]);
        mock().expectOneCall("OUT_GPIO").withParameter("pin", expected_pins[idx]);
        idx++;
    }
    mock().expectOneCall("INP_GPIO").withParameter("pin", RESULT_PIN);
    mock().expectOneCall("INP_GPIO").withParameter("pin", MISO_PIN);
    mock().expectOneCall("INP_GPIO").withParameter("pin", CLK_IN_PIN);
    init_pins(NULL);
}
