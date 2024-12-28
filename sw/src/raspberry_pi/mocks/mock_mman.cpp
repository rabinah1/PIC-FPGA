#include "../../ut/cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "mock_mman.h"
}

void *mmap(void *addr, int size, int opts, int shared, int fd, unsigned int base)
{
    return mock().actualCall("mmap").withParameter("addr", addr).withParameter("size", size).
           withParameter("opts", opts).withParameter("shared", shared).withParameter("fd", fd).
           withParameter("base", base).returnValue().getPointerValue();
}
