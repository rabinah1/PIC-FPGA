#include "../../ut/cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "mock_gpio.h"
}

void INP_GPIO(int pin)
{
    mock().actualCall("INP_GPIO").withParameter("pin", pin);
}

void OUT_GPIO(int pin)
{
    mock().actualCall("OUT_GPIO").withParameter("pin", pin);
}
