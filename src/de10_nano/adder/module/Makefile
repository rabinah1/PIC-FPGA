#obj-m:=hello.o
obj-m:=adder_driver.o

all:
	make ARCH=arm CROSS_COMPILE=/opt/linaro/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- \
	-C ../../../../socfpga-4.5/linux-socfpga/lib/modules/4.5.0+/build M=/home/henry/projects/PIC-FPGA/src/de10_nano/adder/module modules

clean:
	make ARCH=arm CROSS_COMPILE=/opt/linaro/gcc-linaro-4.8-2015.06-x86_64_arm-linux-gnueabihf/bin/arm-linux-gnueabihf- \
	-C ../../../../socfpga-4.5/linux-socfpga/lib/modules/4.5.0+/build M=/home/henry/projects/PIC-FPGA/src/de10_nano/adder/module clean
