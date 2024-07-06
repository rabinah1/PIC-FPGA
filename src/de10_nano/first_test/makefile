SHELL = /bin/bash
TARGET_BINARY = main
CFLAGS = -Wall
CC = /opt/linaro/gcc_12_2/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf-gcc

.PHONY: clean

${TARGET_BINARY}: main.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${TARGET_BINARY} *.a *.o *~
