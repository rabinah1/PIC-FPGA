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
	${CC} -c -I$(SRC_DIR) $(SRC_DIR)/functions.c -o $(SRC_DIR)/functions.o

build: functions.o
	${CC} ${CFLAGS} ${LDFLAGS} ${SRC_DIR}/functions.o ${SRC_DIR}/main.c -o ${TARGET_EXEC}

all: build test_run

clean: test_clean
	rm ${SRC_DIR}/*.o
	rm -f main
