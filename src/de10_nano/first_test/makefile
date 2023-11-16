SHELL = /bin/bash
TARGET_BINARY = main
CFLAGS = -Wall
CC = /opt/linaro/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf-gcc

.PHONY: clean

${TARGET_BINARY}: main.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${TARGET_BINARY} *.a *.o *~
