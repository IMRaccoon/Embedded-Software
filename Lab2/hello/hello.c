#include <linux/init.h>
#include <linux/module.h>
MODULE_LICENSE("Dual BSD/GPL");

static int __init hello_init(void)
{
    printk(KERN_NOTICE, "Hello, World #2\n");
    return 0;
}

static void __exit hello_exit(void)
{
    printk(KERN_NOTICE, "Goodbyt\n");
}

module_init(hello_init);
module_exit(hello_exit);