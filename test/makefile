PROJECT_DIR=/home/henry/projects/PIC-FPGA
SRC_DIR=$(PROJECT_DIR)/src/raspberry_pi
TEST_DIR=$(PROJECT_DIR)/test
# specify where the source code and includes are located
INCLUDE_DIRS=$(SRC_DIR)
SRC_DIRS=$(SRC_DIR)
MOCKS_SRC_DIRS=$(SRC_DIR)/mocks
# specify where the test code is located
TEST_SRC_DIRS=$(TEST_DIR)
# what to call the test binary
TEST_TARGET=PIC16F84A_tests
COMPONENT_NAME=PIC16F84A
# where the cpputest library is located
CPPUTEST_HOME=$(PROJECT_DIR)/cpputest
CPPUTEST_USE_VPATH=Y
CPPUTEST_USE_MEM_LEAK_DETECTION=N
CPPUTEST_USE_EXTENSIONS=Y
CPPUTEST_WARNINGFLAGS=-Wall -Wextra -Wshadow -Wswitch-default -Wswitch-enum -Wconversion -Wno-long-long -Wno-write-strings
# run MakefileWorker.mk with the variables defined here
include MakefileWorker.mk
