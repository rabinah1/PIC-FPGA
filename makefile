TARGET_EXEC := main
SRC_DIR := ./src
TEST_DIR := ./test
LDFLAGS = -lm -pthread -lwiringPi
CC = C:\SysGCC\raspberry\bin\arm-linux-gnueabihf-gcc.exe
CFLAGS = -Wall

test_run:
	make -C ${TEST_DIR}

test_clean:
	make -C ${TEST_DIR} clean

functions.o:
	${CC} -c $(SRC_DIR)/functions.c -o $(SRC_DIR)/functions.o

gpio_setup.o:
	${CC} -c ${SRC_DIR}/gpio_setup.c -o ${SRC_DIR}/gpio_setup.o

arduino.o:
	${CC} -c $(SRC_DIR)/arduino.c -o $(SRC_DIR)/arduino.o

fpga_if.o:
	${CC} -c $(SRC_DIR)/fpga_if.c -o $(SRC_DIR)/fpga_if.o

build: functions.o gpio_setup.o arduino.o fpga_if.o
	${CC} ${CFLAGS} ${LDFLAGS} ${SRC_DIR}/functions.o ${SRC_DIR}/gpio_setup.o ${SRC_DIR}/arduino.o ${SRC_DIR}/fpga_if.o ${SRC_DIR}/app.c -o ${TARGET_EXEC}

all: build test_run

clean: test_clean
	rm ${SRC_DIR}/*.o
	rm -f main
