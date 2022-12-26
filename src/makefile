TARGET_EXEC = main
LDFLAGS = -lm -pthread -lwiringPi
CC = C:\SysGCC\raspberry\bin\arm-linux-gnueabihf-gcc.exe
CFLAGS = -Wall
OBJECTS = functions.o gpio_setup.o arduino.o fpga_if.o app.o

main: $(OBJECTS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(OBJECTS) -o $(TARGET_EXEC)

app.o: gpio_setup.h functions.h defines.h fpga_if.h

functions.o: functions.h defines.h

gpio_setup.o: gpio_setup.h defines.h

arduino.o: arduino.h

fpga_if.o: fpga_if.h defines.h functions.h arduino.h
