#include "/home/henry/projects/PIC-FPGA/cpputest/include/CppUTest/CommandLineTestRunner.h"
extern "C"
{
    #include "wiringPi.h"
}

int main(int ac, char **av)
{
    return CommandLineTestRunner::RunAllTests(ac, av);
}
