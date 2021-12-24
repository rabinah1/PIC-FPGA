# PIC-FPGA

## General information

This project is an implementation of a simple microcontroller on the Intel Cyclone V FPGA, using the Terasic DE10-nano development board. The microcontroller desing is based on the Microchip PIC16F84A microcontroller, but it is not an exact copy of that microcontroller. Software for controlling the FPGA is implemented on the Raspberry Pi 4 model B, and the DE10-nano and Raspberry Pi boards are connected via the GPIO-pins. There is also an Arduino Nano microcontroller connected to the Raspberry Pi, and it can be controlled via the software on the Raspberry Pi.