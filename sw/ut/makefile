SHELL = /bin/bash
CC = gcc
CPPUTEST_HOME = ../cpputest
CPPFLAGS += -I$(CPPUTEST_HOME)/include
OBJ_DIR = ./objs
SRC_DIR = ../src/raspberry_pi
LOGGER_DIR = $(SRC_DIR)/logger/src
MOCK_DIR = $(SRC_DIR)/mocks
LDFLAGS += -L$(CPPUTEST_HOME)/lib -I$(SRC_DIR) -I$(MOCK_DIR)
OBJECTS = $(addprefix $(OBJ_DIR)/, mock_fcntl.o mock_gpio.o mock_hw_if.o mock_mman.o mock_wiringPi.o \
	mock_wiringSerial.o log.o process_command.o common_data.o gpio_setup.o hw_if.o test_common_data.o \
	test_gpio_setup.o test_hw_if.o test_process_command.o main.o)
CPPUTEST_LIBS = cpputest/src/CppUTest/libCppUTest.a cpputest/src/CppUTest/libCppUTestExt.a

.PHONY: clean

all: PIC_FPGA

cpputest:
	./install_cpputest.sh

PIC_FPGA: cpputest $(OBJECTS)
	$(CXX) $(OBJECTS) $(CPPUTEST_LIBS) -o PIC_FPGA $(CPPFLAGS) $(CFLAGS) $(LDLIBS) $(LDFLAGS)

$(OBJECTS): | $(OBJ_DIR)

$(OBJ_DIR):
	mkdir $(OBJ_DIR)

$(OBJ_DIR)/main.o:

$(OBJ_DIR)/test_common_data.o: $(SRC_DIR)/defines.h $(SRC_DIR)/common_data.h

$(OBJ_DIR)/test_gpio_setup.o: $(SRC_DIR)/defines.h $(SRC_DIR)/gpio_setup.h

$(OBJ_DIR)/test_hw_if.o: $(SRC_DIR)/defines.h $(SRC_DIR)/hw_if.h

$(OBJ_DIR)/test_process_command.o: $(SRC_DIR)/defines.h $(SRC_DIR)/process_command.h $(SRC_DIR)/common_data.h

mock_fcntl.o: $(MOCK_DIR)/mock_fcntl.h

mock_gpio.o: $(MOCK_DIR)/mock_gpio.h

mock_hw_if.o: $(SRC_DIR)/hw_if.h

mock_mman.o: $(MOCK_DIR)/mock_mman.h

mock_wiringPi.o: $(MOCK_DIR)/mock_wiringPi.h

mock_wiringSerial.o: $(MOCK_DIR)/mock_wiringSerial.h

log.o: $(LOGGER_DIR)/log.h

common_data.o: $(SRC_DIR)/common_data.h $(SRC_DIR)/defines.h

gpio_setup.o: $(SRC_DIR)/gpio_setup.h $(SRC_DIR)/defines.h

hw_if.o: $(SRC_DIR)/hw_if.h $(SRC_DIR)/defines.h $(SRC_DIR)/common_data.h

process_command.o: $(SRC_DIR)/defines.h $(SRC_DIR)/common_data.h $(SRC_DIR)/hw_if.h

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) -DUNIT_TEST=1 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@

$(OBJ_DIR)/%.o: $(LOGGER_DIR)/%.c
	$(CC) -DUNIT_TEST=1 $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@

$(OBJ_DIR)/%.o: $(MOCK_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@

$(OBJ_DIR)/%.o: %.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -c $< -o $@

clean:
	-rm -rf $(OBJ_DIR)
	-rm -f PIC_FPGA
