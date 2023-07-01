SHELL = /bin/bash
POWERSHELL = C:/Windows/system32/WindowsPowerShell/v1.0/powershell.exe C:/intelFPGA/18.0/embedded/Embedded_Command_Shell.bat
TARGET = hps_main

ALT_DEVICE_FAMILY ?= soc_cv_av
SOCEDS_ROOT ?= $(SOCEDS_DEST_ROOT)
HWLIBS_ROOT = $(SOCEDS_ROOT)/ip/altera/hps/altera_hps/hwlib
CROSS_COMPILE = arm-linux-gnueabihf-
CFLAGS = -g -Wall -D$(ALT_DEVICE_FAMILY) -I$(HWLIBS_ROOT)/include/$(ALT_DEVICE_FAMILY) -I$(HWLIBS_ROOT)/include/
LDFLAGS = -g -Wall 
CC = $(CROSS_COMPILE)gcc
ARCH = arm

build: $(TARGET)
$(TARGET): main.o 
	@POWERSHELL $(CC) $(LDFLAGS) $^ -o $@
%.o : %.c
	@POWERSHELL $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) *.a *.o *~
