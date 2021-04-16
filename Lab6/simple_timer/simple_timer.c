#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

struct my_timer_info
{
    struct timer_list timer;
    long delay_jiffies;
    int data;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t)
{
    struct my_timer_info *info = from_timer(info, t, timer);
    printk("simple_timer: Jiffies %ld, Data %d\n", jiffies, info->data);
    info->data++;
    mod_timer(&my_timer.timer, jiffies + info->delay_jiffies);
}

static int __init simple_init(void)
{
    printk("simple_timer: init module\n");

    my_timer.delay_jiffies = msecs_to_jiffies(2000);
    my_timer.data = 100;
    timer_setup(&my_timer.timer, my_timer_func, 0);
    my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
    add_timer(&my_timer.timer);

    return 0;
}

static void __exit simple_exit(void)
{
    printk("simple_timer: exit module\n");

    del_timer(&my_timer.timer);
}

module_init(simple_init);
module_exit(simple_exit);