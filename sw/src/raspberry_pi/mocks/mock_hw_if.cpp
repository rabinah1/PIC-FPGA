#include "../../ut/cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "hw_if.h"
}

void set_gpio_high(int pin, volatile unsigned *gpio)
{
    mock().actualCall("set_gpio_high").withParameter("pin", pin).withParameter("gpio", gpio);
}

void set_gpio_low(int pin, volatile unsigned *gpio)
{
    mock().actualCall("set_gpio_low").withParameter("pin", pin).withParameter("gpio", gpio);
}
