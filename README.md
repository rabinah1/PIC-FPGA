# PIC-FPGA

## Overview

This is a hobby project for learning VHDL programming and HW/SW co-operation. The project consists roughly of the following three parts:

1. A simple microcontroller implemented on the Terasic DE10-nano development board, and a control SW running on the Raspberry Pi 4 model B which is used to control the implemented microcontroller. The microcontroller is loosely based on the Microchip PIC16F84A microcontroller.
2. A simple software running on the Arduino Nano development board. The software can be invoked from the Raspberry Pi.
3. A simple adder implemented on the DE10-nano FPGA, and an associated control SW combined with a kernel module which are running on the SoC of the DE10-nano. After loading the kernel module, the SW can be used to send two numbers to the HW adder, and the sum of these numbers is returned.

Raspberry Pi and Terasic DE10-nano are connected to each other via the GPIO pins. The Arduino Nano is connected to the Raspberry Pi via a USB cable. More details about the connections are provided in the "Hardware setup" section.

## Repository structure

Below is a short description of the repository structure.

- doc/
    - Technical documents.
- hw/
    - Hardware source code, testbench and style check configuration.
- sw/
    - Software source code and unit tests.
- test_data/
    - Input data files for testing, and helper scripts for test automation.
- makefile: Top-level makefile.
- run_tests.sh: Script for running test on real HW.
- setup_env.sh: Environment setup script for GPIO usage. This must be executed on the Raspberry Pi before running the SW.

## Hardware setup

This section describes the HW setup, including list of required components and connections between them.

List of components:

- 1 x Breadboard
- 1 x Terasic DE10-nano development board
- 1 x Raspberry Pi 4 model B development board
- 1 x Arduino Nano development board
- 1 x PCF8582-E eeprom chip
- 1 x TMP36 temperature sensor
- 7 x 100K resistors
- 2 x 5K resistors
- 1 x USB A to mini USB B cable
- Jumper wires

Below is a circuit diagram of system:

![plot](./doc/Schematic.png?raw=true "Circuit schematic")