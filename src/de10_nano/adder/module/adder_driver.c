#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/uaccess.h>
#include <linux/ioport.h>
#include <linux/io.h>
#include <linux/slab.h>
#include <linux/string.h>

// https://zhehaomao.com/blog/fpga/2013/12/29/sockit-4.html
// There should be a file /sys/bus/platform/drivers/adder/adder

#define LWH2F_BRIDGE_BASE 0xFF200000
#define MMAP_LENGTH 4096
#define OPERAND_1_OFFSET 0x0
#define OPERAND_2_OFFSET 0x2
#define RESULT_OFFSET 0x4

void *adder_mem;

static struct device_driver adder_driver = {
    .name = "adder",
    .bus = &platform_bus_type // https://www.kernel.org/doc/Documentation/driver-model/platform.txt
};

ssize_t adder_read(struct device_driver *drv, char __user *buf)
{
    u16 sum;
    char sum_str[10];
    printk("adder_read entered\n");
    sum = ioread16(adder_mem + RESULT_OFFSET);

    if (sum & 1 << 0xF) {
        sum = sum & 0x7FFF;
    }

    memset(sum_str, '\0', sizeof(sum_str));
    sprintf(sum_str, "%u", sum);
    printk("sum_str = %s\n", sum_str);
    memcpy(buf, sum_str, strlen(sum_str));
    return sizeof(sum_str);
}

ssize_t adder_write(struct device_driver *drv, const char *buf, size_t count)
{
    u16 operand_1 = 0;
    u16 operand_2 = 0;
    char *string = NULL;
    char *string_2 = NULL;
    char *num_1 = NULL;
    char num_1_arr[10];
    char num_2_arr[10];
    char *num_2 = NULL;
    printk("adder_write entered\n");

    if (buf == NULL) {
        pr_err("Error, string must not be NULL\n");
        return -EINVAL;
    }

    memset(num_1_arr, '\0', sizeof(num_1_arr));
    memset(num_2_arr, '\0', sizeof(num_2_arr));
    string = kmalloc(sizeof(buf), GFP_KERNEL);
    memset(string, '\0', sizeof(string));

    if (!string)
        return 0;

    strcpy(string, buf);
    string_2 = string;
    num_1 = strsep(&string_2, " ");
    strcpy(num_1_arr, num_1);
    num_2 = strsep(&string_2, " ");
    strcpy(num_2_arr, num_2);
    kfree(string);

    if (kstrtou16(num_1_arr, 10, &operand_1) < 0) {
        pr_err("Could not convert string 1 to integer\n");
        return -EINVAL;
    }

    if (kstrtou16(num_2_arr, 10, &operand_2) < 0) {
        pr_err("Could not convert string 2 to integer\n");
        return -EINVAL;
    }

    iowrite16(operand_1, adder_mem + OPERAND_1_OFFSET);
    iowrite16(operand_2, adder_mem + OPERAND_2_OFFSET);
    printk("iowrite calls done\n");
    return 0;
}

// DRIVER_ATTR is a helper to create a driver_attr struct. Arguments are name, permission mode, show function and store function.
// When name is "adder", this will declare a struct called "driver_attr_adder".
// static DRIVER_ATTR(adder, (S_IWUSR | S_IRUGO), adder_read, adder_write);
struct driver_attribute driver_attr_adder = {
    .attr   = { .name = "adder", .mode = (S_IWUSR | S_IRUGO) },
    .show   = adder_read,
    .store  = adder_write
};

MODULE_LICENSE("Dual BSD/GPL");

static int __init adder_init(void)
{
    int ret;
    struct resource *res;
    ret = driver_register(&adder_driver);

    if (ret < 0)
        return ret;

    printk("Driver registered\n");
    ret = driver_create_file(&adder_driver, &driver_attr_adder);

    if (ret < 0) {
        pr_err("Failed to register adder_driver\n");
        driver_unregister(&adder_driver);
        return ret;
    }

    printk("driver_create_file succeeded\n");
    // Request the "raw" memory region
    res = request_mem_region(LWH2F_BRIDGE_BASE, MMAP_LENGTH, "adder");

    if (res == NULL) {
        pr_err("Requesting memory region for adder_driver failed\n");
        driver_remove_file(&adder_driver, &driver_attr_adder);
        driver_unregister(&adder_driver);
        return -EBUSY;
    }

    printk("Raw mem region requested\n");
    // Map the raw address to virtual memory
    adder_mem = ioremap(LWH2F_BRIDGE_BASE, MMAP_LENGTH);

    if (adder_mem == NULL) {
        pr_err("Mapping physical address to virtual address failed\n");
        driver_remove_file(&adder_driver, &driver_attr_adder);
        driver_unregister(&adder_driver);
        release_mem_region(LWH2F_BRIDGE_BASE, MMAP_LENGTH);
        return -EFAULT;
    }

    printk("ioremap succeeded\n");
    return 0;
}

static void __exit adder_exit(void)
{
    printk("Removing module");
    driver_remove_file(&adder_driver, &driver_attr_adder);
    driver_unregister(&adder_driver);
    release_mem_region(LWH2F_BRIDGE_BASE, MMAP_LENGTH);
    iounmap(adder_mem);
}

module_init(adder_init);
module_exit(adder_exit);
