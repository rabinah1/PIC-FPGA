# PIC-FPGA

## General information

This is a hobby project for learning VHDL programming, and HW/SW co-operation. The used HW consists of

- Terasic DE10-nano development board
- Rasbperry Pi 4 model B
- Arduino Nano
- PCF8582-E EEPROM chip

The project mainly consists of a simple microcontroller implemented on the Terasic DE10-nano (using Intel Cyclone V FPGA), and a control SW running on the Raspberry Pi, which is used to control the implemented microcontroller. The microcontroller is based on the Microchip PIC16F84A microcontroller. Raspberry Pi and Terasic DE-10 nano are connected to each other via the GPIO pins. The PCF8582-E chip is also connected to the DE10-nano via the GPIO pins. The Arduino Nano is connected to the Raspberry Pi via USB cable, and it can be controller via the software on the Raspberry Pi.

## Repository structure

- cpputest/
    - Cpputest submodule.
- doc/
    - Technical documents.
- src/
    - Software source code.
- test/
    - Software unit tests.
- test_data/
    - Reference and input data for test automation, and Python-scripts for processing the data.
- vhdl_src/
    - VHDL source code.
- install_cpputest.sh: Script to install cpputest.
- makefile: Top-level makefile that is used for both SW and HW.
- run_tests.sh: Script for running test on real HW.
- setup_env.sh: Environment setup script for GPIO usage.
- test_file.txt: File that can be read by the Raspberry Pi, all commands in it are executed.
