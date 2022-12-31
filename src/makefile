TARGET_EXEC = main
LDFLAGS = -lm -pthread -lwiringPi
CC = C:\SysGCC\raspberry\bin\arm-linux-gnueabihf-gcc.exe
CFLAGS = -Wall
OBJECTS = process_command.o common_data.o gpio_setup.o hw_if.o app.o

.DELETE_ON_ERROR:

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET_EXEC)

app.o: gpio_setup.h common_data.h defines.h hw_if.h process_command.h

common_data.o: common_data.h defines.h

gpio_setup.o: gpio_setup.h defines.h

hw_if.o: hw_if.h defines.h common_data.h

process_command.o: defines.h common_data.h hw_if.h
