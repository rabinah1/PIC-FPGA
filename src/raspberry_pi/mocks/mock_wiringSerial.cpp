#include "../../cpputest/include/CppUTestExt/MockSupport.h"

extern "C"
{
#include "mock_wiringSerial.h"
}

int serialOpen(char *port, int baud_rate)
{
    return mock().actualCall("serialOpen").withParameter("serial_port", port).
           withParameter("baud_rate", baud_rate).returnIntValue();
}

void serialPuts(int fd, char *command)
{
    mock().actualCall("serialPuts").withParameter("fd", fd).withParameter("command", command);
}

int serialDataAvail(int fd)
{
    return mock().actualCall("serialDataAvail").withParameter("fd", fd).returnIntValue();
}

int serialGetchar(int fd)
{
    return mock().actualCall("serialGetchar").withParameter("fd", fd).returnIntValue();
}

void serialClose(int fd)
{
    mock().actualCall("serialClose").withParameter("fd", fd);
}
