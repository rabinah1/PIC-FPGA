PROJECT_DIR=C:/Users/henry/PIC-FPGA
SRC_DIR=$(PROJECT_DIR)/src
TEST_DIR=$(PROJECT_DIR)/test
# specify where the source code and includes are located
INCLUDE_DIRS=$(SRC_DIR)
SRC_FILES=$(SRC_DIR)/functions.c
#SRC_DIRS=$(SRC_DIR)
# specify where the test code is located
TEST_SRC_DIRS=$(TEST_DIR)
# what to call the test binary
TEST_TARGET=PIC16F84A_tests
COMPONENT_NAME=PIC16F84A
# where the cpputest library is located
CPPUTEST_HOME=$(PROJECT_DIR)/cpputest
CPPUTEST_USE_VPATH=Y
CPPUTEST_USE_MEM_LEAK_DETECTION=N
CPPUTEST_WARNINGFLAGS=-Wall -Wextra -Wshadow -Wswitch-default -Wswitch-enum -Wconversion -Wno-long-long -Wno-write-strings
# run MakefileWorker.mk with the variables defined here
include MakefileWorker.mk
