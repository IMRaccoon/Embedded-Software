#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

MODULE_LICENSE("GPL");

#define DEV_NAME "ch2_dev"

#define IOCTL_START_NUM 1
#define IOCTL_NUM_GET IOCTL_START_NUM + 1
#define IOCTL_NUM_SET IOCTL_START_NUM + 2
#define IOCTL_NUM_ADD IOCTL_START_NUM + 3
#define IOCTL_NUM_MUL IOCTL_START_NUM + 4

#define CH_IOCTL_MAGIC 'z'
#define GET_IOCTL _IOR(CH_IOCTL_MAGIC, IOCTL_NUM_GET, long)
#define SET_IOCTL _IOW(CH_IOCTL_MAGIC, IOCTL_NUM_SET, long)
#define ADD_IOCTL _IOW(CH_IOCTL_MAGIC, IOCTL_NUM_ADD, long)
#define MUL_IOCTL _IOW(CH_IOCTL_MAGIC, IOCTL_NUM_MUL, long)

static long result = 0;

static int ch2_dev_open(struct inode *inode, struct file *file)
{
    printk("ch2_dev: open\n");
    return 0;
}

static int ch2_dev_release(struct inode *inode, struct file *file)
{
    printk("ch2_dev: close\n");
    return 0;
}

static long ch2_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case GET_IOCTL:
        printk("ch2_dev: get\n");
        return result;
    case SET_IOCTL:
        printk("ch2_dev: set %ld to %ld\n", result, arg);
        result = arg;
        break;
    case ADD_IOCTL:
        printk("ch2_dev: add %ld and %ld\n", result, arg);
        result += arg;
        break;
    case MUL_IOCTL:
        printk("ch2_dev: mul %ld and %ld\n", result, arg);
        result *= arg;
        break;
    default:
        return -1;
    }
    return 0;
}

struct file_operations ch2_dev_fops = {
    .open = ch2_dev_open,
    .release = ch2_dev_release,
    .unlocked_ioctl = ch2_dev_ioctl,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init ch2_dev_init(void)
{
    printk("ch2_dev: init module\n");

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ch2_dev_fops);
    cdev_add(cd_cdev, dev_num, 1);

    return 0;
}

static void __exit ch2_dev_exit(void)
{
    printk("ch2_dev: exit module\n");

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
}

module_init(ch2_dev_init);
module_exit(ch2_dev_exit);