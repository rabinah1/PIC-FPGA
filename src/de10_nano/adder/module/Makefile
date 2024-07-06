#obj-m:=hello.o
obj-m:=adder_driver.o

all:
	make ARCH=arm CROSS_COMPILE=/opt/linaro/gcc_12_2/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf- \
	-C /home/henry/kernels/linux-socfpga M=/home/henry/projects/PIC-FPGA/src/de10_nano/adder/module modules

clean:
	make ARCH=arm CROSS_COMPILE=/opt/linaro/gcc_12_2/arm-gnu-toolchain-12.2.rel1-x86_64-arm-none-linux-gnueabihf/bin/arm-none-linux-gnueabihf- \
	-C /home/henry/kernels/linux-socfpga M=/home/henry/projects/PIC-FPGA/src/de10_nano/adder/module clean
