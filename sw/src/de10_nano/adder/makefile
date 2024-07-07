SHELL = /bin/bash
TARGET_BINARY = main
CFLAGS = -Wall
CC = $(DE10_CROSS)gcc

.PHONY: clean

${TARGET_BINARY}: main.o
	$(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${TARGET_BINARY} *.a *.o *~
