#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("Invalid number of arguments.\n");
        return 1;
    }

    char *operand_1 = argv[1];
    char *operand_2 = argv[2];
    char operands[10];
    sprintf(operands, "%s %s", operand_1, operand_2);
    int fd = open("/sys/bus/platform/drivers/adder/adder", O_RDWR);
    ssize_t ret = write(fd, &operands[0], sizeof(operands));
    printf("Bytes written = %d\n", ret);
    sleep(1);
    ret = read(fd, &operands[0], sizeof(operands));
    sleep(1);
    printf("Bytes read = %d\n", ret);
    printf("Value read = %s\n", operands);
    close(fd);
    return 0;
}

// Modify uts_release in lib/modules/4.5.0+/source/include/generated/utsrelease.h
