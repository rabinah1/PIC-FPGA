#obj-m:=hello.o
obj-m:=adder_driver.o

all:
	make ARCH=arm CROSS_COMPILE=$(DE10_CROSS) -C $(DE10_KERNEL_SRC) M=$(shell pwd) modules

clean:
	make ARCH=arm CROSS_COMPILE=$(DE10_CROSS) -C $(DE10_KERNEL_SRC) M=$(shell pwd) clean
