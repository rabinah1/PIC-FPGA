SHELL = /bin/bash
TARGET_BINARY = hps_main
POWERSHELL = C:/Windows/system32/WindowsPowerShell/v1.0/powershell.exe C:/intelFPGA/18.0/embedded/Embedded_Command_Shell.bat
CFLAGS = -Wall
CC = arm-linux-gnueabihf-gcc

.PHONY: clean

${TARGET_BINARY}: main.o
	@POWERSHELL $(CC) $(CFLAGS) $^ -o $@

main.o: main.c
	@POWERSHELL $(CC) $(CFLAGS) -c $< -o $@

clean:
	-rm -f ${TARGET_BINARY} *.a *.o *~
