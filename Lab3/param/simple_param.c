#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include "simple_param.h"

MODULE_LICENSE("GPL");

static long my_id = 0;
module_param(my_id, long, 0);

static int simple_param_open(struct inode *inode, struct file *file)
{
    printk("simple_param: open\n");
    return 0;
}

static int simple_param_release(struct inode *inode, struct file *file)
{
    printk("simple_param: release\n");
    return 0;
}

static long simple_param_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case PARAM_GET:
        printk("simple_param: return my_id %ld \n", my_id);
        return my_id;
    case PARAM_SET:
        printk("simple_param: set my_id %ld to %ld \n", my_id, (long)arg);
        my_id = (long)arg;
        return my_id;
    default:
        return -1;
    }

    return 0;
}

struct file_operations simple_param_fops = {
    .open = simple_param_open,
    .release = simple_param_release,
    .unlocked_ioctl = simple_param_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init simple_param_init(void)
{
    printk("simple_param: init module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &simple_param_fops);
    cdev_add(cd_cdev, dev_num, 1);

    return 0;
}

static void __exit simple_param_exit(void)
{
    printk("simple_param: exit module\n");

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
}

module_init(simple_param_init);
module_exit(simple_param_exit);