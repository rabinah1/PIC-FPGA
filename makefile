TARGET_EXEC := PIC16F84A
SRC_DIR := ./src
TEST_DIR := ./test
LDFLAGS = -lm -pthread -lwiringPi
CC = gcc
CFLAGS = -Wall

test_run:
	make -C ${TEST_DIR}

test_clean:
	make -C ${TEST_DIR} clean

code.o:
	${CC} -c -I$(SRC_DIR) $(SRC_DIR)/code.c -o $(SRC_DIR)/code.o

PIC16F84A: code.o
	sudo ${CC} ${CFLAGS} ${LDFLAGS} ${SRC_DIR}/code.o ${SRC_DIR}/PIC16F84A.c -o ${TARGET_EXEC}

all: PIC16F84A test_run

clean: test_clean
	rm ${SRC_DIR}/*.o
	rm -f PIC16F84A
