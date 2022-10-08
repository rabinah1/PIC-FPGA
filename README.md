# PIC-FPGA

## General information

This project is an implementation of a simple microcontroller on the Intel Cyclone V FPGA, using the Terasic DE10-nano development board. The microcontroller design is based on the Microchip PIC16F84A microcontroller, but it is not an exact copy of that microcontroller. Non-volatile memory is implemented with a PCF8582E-2 EEPROM chip, which is connected to the FPGA via the GPIO-pins. Software for controlling the FPGA is implemented on the Raspberry Pi 4 model B, and the DE10-nano and Raspberry Pi boards are connected via the GPIO-pins. There is also an Arduino Nano microcontroller connected to the Raspberry Pi via USB cable, and it can be controlled via the software on the Raspberry Pi.

## Repository structure

- src/
    - Software source code.
- vhdl_src/
    - VHDL source code.
- test/
    - Software unit tests.
- test_data/
    - Reference and input data for test automation, and python-scripts for processing the data.
- setup_env.sh: Environment setup script for GPIO usage.
- run_tests.sh: Script for running test on real HW.
- fpga_tool.ps1: Script for building the FPGA design, running tests using Modelsim, and loading the bitfile to the FPGA.
- test_file.txt: File that can be read by the Raspberry Pi, all commands in it are executed.
- vsim_commands.txt: List of waves that are added to the Modelsim simulation.
- vsg_config.json: Configuration file for VHDL style check (VSG).

## Static analysis

- Run Pylint: pylint <file.py>
- Run flake8: flake8 <file.py> --max-line-length=100
