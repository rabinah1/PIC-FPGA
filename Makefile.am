SHELL = /usr/bin/bash
SUBDIRS = src

.DELETE_ON_ERROR:
.PHONY: all clean sw hw build_sw check_sw sta_sw clean_sw build_arduino load_arduino build_hw check_hw sta_hw netlist load_hw clean_hw help

all: sw hw

sw: build_sw build_arduino check_sw sta_sw

hw: build_hw check_hw sta_hw

clean: clean_hw clean_sw

build_sw:
	@echo "Compiling project..."
	$(MAKE) -C $(top_srcdir)/build/src
	$(MAKE) -C $(top_srcdir)/src/hps
	@echo "Done"
	@echo ""

check_sw:
	@echo "Running tests..."
	$(MAKE) -C $(top_srcdir)/test
	@echo "Done"
	@echo ""

sta_sw:
	@echo "Runnig Astyle..."
	@astyle --style=linux --max-code-length=100 --recursive --align-pointer=name --break-blocks \
	--pad-oper --pad-header --delete-empty-lines --indent-col1-comments --squeeze-lines=1 \
	--exclude="$(top_srcdir)\src\arduino\build" -i "$(top_srcdir)\src\*.c,*.cpp,*.h" "$(top_srcdir)\test\*.cpp"
	@echo "Done"
	@echo ""
	@echo "Running pylint..."
	@pylint $(top_srcdir)/test_data/*.py
	@echo "Done"
	@echo ""
	@echo "Running flake8..."
	@flake8 $(top_srcdir)/test_data/*.py --max-line-length=100
	@echo "Done"

clean_sw:
	$(MAKE) -C $(top_srcdir)/test clean
	-rm -f $(top_srcdir)/test/main
	-rm -rf $(top_srcdir)/src/objs
	-rm -rf $(top_srcdir)/src/arduino/build
	-rm -rf $(top_srcdir)/autom4te.cache
	-rm -f $(top_srcdir)/src/hps/main.o
	-rm -f $(top_srcdir)/src/hps/hps_main
	-rm -f $(top_srcdir)/install-sh
	-rm -f $(top_srcdir)/Makefile.in
	-rm -f $(top_srcdir)/aclocal.m4
	-rm -f $(top_srcdir)/compile
	-rm -f $(top_srcdir)/config.h.in
	-rm -f $(top_srcdir)/configure
	-rm -f $(top_srcdir)/depcomp
	-rm -f $(top_srcdir)/missing
	-rm -f $(top_srcdir)/src/Makefile.in
	-rm -f $(top_srcdir)/configure~

sta_hw:
	$(MAKE) -C $(top_srcdir)/vhdl_src sta

build_arduino:
	@echo "Building Arduino design..."
	arduino-cli compile --build-path $(top_srcdir)/src/arduino/build --fqbn \
	arduino:avr:nano:cpu=atmega328 $(top_srcdir)/src/arduino/arduino.ino
	@echo "Done"

load_arduino:
	@echo "Loading design to Arduino..."
	arduino-cli upload --input-dir $(top_srcdir)/src/arduino/build -p $(PORT) --fqbn \
	arduino:avr:nano:cpu=atmega328 $(top_srcdir)/src/arduino/arduino.ino
	@echo "Done"

build_hw:
	$(MAKE) -C $(top_srcdir)/vhdl_src build

check_hw:
	$(MAKE) -C $(top_srcdir)/vhdl_src check

load_hw:
	$(MAKE) -C $(top_srcdir)/vhdl_src load

netlist:
	$(MAKE) -C $(top_srcdir)/vhdl_src netlist

clean_hw:
	$(MAKE) -C $(top_srcdir)/vhdl_src clean

help:
	@echo "Available targets for this makefile are:"
	@echo ""
	@echo "'all' (default): Build, run tests and run style checks for both HW and SW designs."
	@echo "'sw': Build, run tests and run style check for SW design."
	@echo "'hw': Build, run tests and run style check for HW design."
	@echo "'build_sw': Build SW design."
	@echo "'check_sw': Run tests for SW design."
	@echo "'sta_sw': Run style check for SW design."
	@echo "'build_arduino': Build the arduino code."
	@echo "'load_arduino': Load the arduino-design to Arduino Nano. You need to give 'PORT = <port>' as an argument."
	@echo "'build_hw': Build HW design."
	@echo "'check_hw' Run tests for HW design. You can set 'TEST_ARGS = -save_test_output' to save the test results."
	@echo "'sta_hw': Run style check for HW design."
	@echo "'netlist': Create and open the netlist for HW design."
	@echo "'load_hw': Load the bitfile to the FPGA. You can set 'LOAD_ARGS = -bitfile <path_to_bitfile>'" \
	"to give a path to the bitfile. The default bitfile is vhdl_src/output_files/PIC16F84A.sof."
	@echo "'clean': Remove all makefile-generated files."
	@echo "'clean_sw': Remove all makefile-generated files for SW design."
	@echo "'clean_hw': Remove all makefile-generated files for HW design."
	@echo "'help': Print this help text."
