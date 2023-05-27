#include "../cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "mock_fcntl.h"
}

int open(char *file, int opts)
{
    return mock().actualCall("open").withParameter("file", file).
           withParameter("opts", opts).returnIntValue();
}
