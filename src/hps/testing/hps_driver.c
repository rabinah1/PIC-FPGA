#include<linux/module.h>
#include<linux/init.h>
#include<linux/kernel.h>

static int __init hello_init(void)
{
    printk(KERN_INFO "Hello example init\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk("Hello example exit\n");
}

module_init(hello_init);
module_exit(hello_exit);

MODULE_AUTHOR("Henry Räbinä");
MODULE_DESCRIPTION("Hello world example");
MODULE_LICENSE("GPL");
