#include "wiringSerial.h"

int serialOpen(char *port, int baud_rate)
{
    (void) port;
    (void) baud_rate;

    return 1;
}

void serialPuts(int fd, char *command)
{
    (void) fd;
    (void) command;

    return;
}

int serialDataAvail(int fd)
{
    (void) fd;

    return 1;
}

char serialGetchar(int fd)
{
    (void) fd;

    return '\0';
}

void serialClose(int fd)
{
    (void) fd;

    return;
}
