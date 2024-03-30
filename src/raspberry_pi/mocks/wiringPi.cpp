#include "../../cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "wiringPi.h"
}

int wiringPiSetup(void)
{
    return mock().actualCall("wiringPiSetup").returnIntValue();
}
