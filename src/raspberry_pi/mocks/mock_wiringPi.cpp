#include "../../cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "mock_wiringPi.h"
}

int wiringPiSetup(void)
{
    return mock().actualCall("wiringPiSetup").returnIntValue();
}
