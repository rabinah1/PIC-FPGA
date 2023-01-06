#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#ifndef UNIT_TEST
#include <wiringSerial.h>
#else
#include "mocks/wiringSerial.h"
#endif

int send_to_arduino(char *command, FILE *result_file, char *serial_port)
{
    int fd;
    if ((fd = serialOpen(serial_port, 9600)) < 0) {
        printf("%s, Failed to open serial connection to port %s, exiting...\n", __func__,
               serial_port);
        return 1;
    }
    sleep(2); // Wait for serial connection to stabilize
    serialPuts(fd, command);

    while (true) {
        if (serialDataAvail(fd) > 0) {
            char input = serialGetchar(fd);
            if (input == '\n') {
                if (result_file != NULL)
                    fprintf(result_file, "%c", input);
                else
                    printf("%c", input);
                break;
            }
            if (result_file != NULL)
                fprintf(result_file, "%c", input);
            else
                printf("%c", input);
        }
    }
    serialClose(fd);
    printf("\n%s, Connection closed\n", __func__);
    return 0;
}
