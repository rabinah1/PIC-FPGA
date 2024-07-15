SHELL = /bin/bash
SW_SRC_DIR = ./sw/src
SW_HW_DIR = ./hw/src
OBJ_DIR = $(SW_SRC_DIR)/raspberry_pi/objs
SW_TEST_DIR = ./sw/ut
TEST_DATA_DIR = ./test_data/scripts/ut

include .env
export

.DELETE_ON_ERROR:
.PHONY: all clean sw hw build_sw check_sw sta_sw clean_sw load_arduino build_hw check_hw sta_hw netlist load_hw clean_hw help

all: sw hw

sw: build_sw check_sw sta_sw

hw: build_hw check_hw sta_hw

clean: clean_hw clean_sw

build_sw:
	@echo "===================================="
	@echo "SW build starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_SRC_DIR)/raspberry_pi
	@$(MAKE) -C $(SW_SRC_DIR)/de10_nano/first_test
	@$(MAKE) -C $(SW_SRC_DIR)/de10_nano/adder
	@$(MAKE) -C $(SW_SRC_DIR)/de10_nano/adder/module
	@arduino-cli compile --build-path $(SW_SRC_DIR)/arduino/build --fqbn \
	arduino:avr:nano:cpu=atmega328 $(SW_SRC_DIR)/arduino/arduino.ino
	@echo ""
	@echo "===================================="
	@echo "SW build completed"
	@echo "===================================="
	@echo ""

check_sw:
	@echo "===================================="
	@echo "SW tests starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_TEST_DIR)
	@$(SW_TEST_DIR)/PIC_FPGA
	pytest $(TEST_DATA_DIR)
	@echo ""
	@echo "===================================="
	@echo "SW tests completed"
	@echo "===================================="
	@echo ""

sta_sw:
	@echo "===================================="
	@echo "SW static analysis starting"
	@echo "===================================="
	@echo ""
	@astyle --style=linux --max-code-length=100 --recursive --align-pointer=name --break-blocks \
	--pad-oper --pad-header --delete-empty-lines --indent-col1-comments --squeeze-lines=1 \
	--exclude="arduino\build" --exclude="de10_nano/adder/module/adder_driver.mod.c" \
	-i "${SW_SRC_DIR}\*.c,*.cpp,*.h" "${SW_TEST_DIR}\*.cpp"
	@echo "Done"
	@echo ""
	@echo "Running pylint..."
	@pylint $(TEST_DATA_DIR)/*.py ./hw/run.py
	@echo "Done"
	@echo ""
	@echo "Running flake8..."
	@flake8 $(TEST_DATA_DIR)/*.py ./hw/run.py --max-line-length=100
	@echo ""
	@echo "===================================="
	@echo "SW static analysis completed"
	@echo "===================================="
	@echo ""

clean_sw:
	@echo "===================================="
	@echo "SW cleanup starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_TEST_DIR) clean
	@-rm -f $(SW_SRC_DIR)/raspberry_pi/main
	@-rm -rf $(OBJ_DIR)
	@-rm -rf $(SW_SRC_DIR)/arduino/build
	@-rm $(SW_SRC_DIR)/de10_nano/first_test/main.o
	@-rm $(SW_SRC_DIR)/de10_nano/first_test/main
	@-rm $(SW_SRC_DIR)/de10_nano/adder/main.o
	@-rm $(SW_SRC_DIR)/de10_nano/adder/main
	@$(MAKE) -C $(SW_SRC_DIR)/de10_nano/adder/module clean
	@echo ""
	@echo "===================================="
	@echo "SW cleanup completed"
	@echo "===================================="
	@echo ""

sta_hw:
	@echo "===================================="
	@echo "HW static analysis starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) sta
	@echo ""
	@echo "===================================="
	@echo "HW static analysis completed"
	@echo "===================================="
	@echo ""

load_arduino:
	@echo "===================================="
	@echo "Loading design to Arduino starting"
	@echo "===================================="
	@echo ""
	@arduino-cli upload --input-dir $(SW_SRC_DIR)/arduino/build -p $(PORT) --fqbn \
	arduino:avr:nano:cpu=atmega328 $(SW_SRC_DIR)/arduino/arduino.ino
	@echo ""
	@echo "===================================="
	@echo "Loading design to Arduino completed"
	@echo "===================================="
	@echo ""

build_hw:
	@echo "===================================="
	@echo "HW build starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) build
	@echo ""
	@echo "===================================="
	@echo "HW build completed"
	@echo "===================================="
	@echo ""

check_hw:
	@echo "===================================="
	@echo "HW tests starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) check
	@echo ""
	@echo "===================================="
	@echo "HW tests completed"
	@echo "===================================="
	@echo ""

load_hw:
	@echo "===================================="
	@echo "Loading HW to FPGA starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) load
	@echo ""
	@echo "===================================="
	@echo "Loading HW to FPGA completed"
	@echo "===================================="
	@echo ""

netlist:
	@echo "===================================="
	@echo "Create netlist starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) netlist
	@echo ""
	@echo "===================================="
	@echo "Create netlist completed"
	@echo "===================================="
	@echo ""

clean_hw:
	@echo "===================================="
	@echo "HW cleanup starting"
	@echo "===================================="
	@echo ""
	@$(MAKE) -C $(SW_HW_DIR) clean
	@echo ""
	@echo "===================================="
	@echo "HW cleanup completed"
	@echo "===================================="
	@echo ""

help:
	@echo "Available targets for this makefile are:"
	@echo ""
	@echo "'all' (default): Build, run tests and run style checks for both HW and SW designs."
	@echo "'sw': Build, run tests and run style check for SW design."
	@echo "'hw': Build, run tests and run style check for HW design."
	@echo "'build_sw': Build SW design."
	@echo "'check_sw': Run tests for SW design."
	@echo "'sta_sw': Run style check for SW design."
	@echo "'load_arduino': Load the arduino-design to Arduino Nano. You need to give 'PORT = <port>' as an argument."
	@echo "'build_hw': Build HW design."
	@echo "'check_hw' Run tests for HW design. You can set 'STORE_RESULT = yes' to save the test results."
	@echo "'sta_hw': Run style check for HW design."
	@echo "'netlist': Create and open the netlist for HW design."
	@echo "'load_hw': Load the bitfile to the FPGA. You can set 'BITFILE = <path_to_bitfile>'" \
	"to give a path to the bitfile. The default bitfile is vhdl_src/output_files/pic16f84a.sof."
	@echo "'clean': Remove all makefile-generated files."
	@echo "'clean_sw': Remove all makefile-generated files for SW design."
	@echo "'clean_hw': Remove all makefile-generated files for HW design."
	@echo "'help': Print this help text."
