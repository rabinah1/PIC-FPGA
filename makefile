SRC_DIR = ./src
VHDL_DIR = ./vhdl_src
OBJ_DIR = $(SRC_DIR)/objs
TEST_DIR = ./test
TEST_DATA_DIR = ./test_data

.DELETE_ON_ERROR:
.PHONY: all clean sw hw build_sw check_sw sta_sw clean_sw build_hw check_hw sta_hw netlist load clean_hw help

all: sw hw

sw: build_sw check_sw sta_sw

hw: build_hw check_hw sta_hw

clean: clean_hw clean_sw

build_sw:
	@echo "Compiling project..."
	$(MAKE) -C $(SRC_DIR)
	@echo "Done"
	@echo ""

check_sw:
	@echo "Running tests..."
	$(MAKE) -C $(TEST_DIR)
	@echo "Done"
	@echo ""

sta_sw:
	@echo "Running pylint..."
	@pylint $(TEST_DATA_DIR)/*.py
	@echo "Done"
	@echo ""
	@echo "Running flake8..."
	@flake8 $(TEST_DATA_DIR)/*.py --max-line-length=100
	@echo "Done"

clean_sw:
	$(MAKE) -C $(TEST_DIR) clean
	-rm -f $(SRC_DIR)/main
	-rm -rf $(OBJ_DIR)

sta_hw:
	$(MAKE) -C $(VHDL_DIR) sta

build_hw:
	$(MAKE) -C $(VHDL_DIR) build

check_hw:
	$(MAKE) -C $(VHDL_DIR) check

load:
	$(MAKE) -C $(VHDL_DIR) load

netlist:
	$(MAKE) -C $(VHDL_DIR) netlist

clean_hw:
	$(MAKE) -C $(VHDL_DIR) clean

help:
	@echo "Available targets for this makefile are:"
	@echo ""
	@echo "'all' (default): Build, run tests and run style checks for both HW and SW designs."
	@echo "'sw': Build, run tests and run style check for SW design."
	@echo "'hw': Build, run tests and run style check for HW design."
	@echo "'build_sw': Build SW design."
	@echo "'check_sw': Run tests for SW design."
	@echo "'sta_sw': Run style check for SW design."
	@echo "'build_hw': Build HW design."
	@echo "'check_hw' Run tests for HW design. You can set 'TEST_ARGS = -save_test_output' to save the test results."
	@echo "'sta_hw': Run style check for HW design."
	@echo "'netlist': Create and open the netlist for HW design."
	@echo "'load': Load the bitfile to the FPGA. You can set 'LOAD_ARGS = -bitfile <path_to_bitfile>' to give a path to the bitfile. The default bitfile is vhdl_src/output_files/PIC16F84A.sof."
	@echo "'clean': Remove all makefile-generated files."
	@echo "'clean_sw': Remove all makefile-generated files for SW design."
	@echo "'clean_hw': Remove all makefile-generated files for HW design."
	@echo "'help': Print this help text."
