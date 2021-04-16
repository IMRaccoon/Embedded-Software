#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>

MODULE_LICENSE("GPL");

#define LED1 5
#define SWITCH 12

struct my_timer_info
{
    struct timer_list timer;
    long delay_jiffies;
    int switch_input;
};

static struct my_timer_info my_timer;

static void my_timer_func(struct timer_list *t)
{
    int ret;
    struct my_timer_info *info = from_timer(info, t, timer);
    ret = gpio_get_value(SWITCH);
    printk("ch5: function activated switch: %d\n", ret);
    if (ret != info->switch_input)
    {
        printk("ch5: change\n");
        gpio_set_value(LED1, ret);
        info->switch_input = ret;
    }
    mod_timer(&info->timer, jiffies + info->delay_jiffies);
}

static int __init simple_led_init(void)
{
    printk("ch5: Hello LED\n");
    gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
    gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

    my_timer.delay_jiffies = msecs_to_jiffies(500);
    my_timer.switch_input = 0;
    timer_setup(&my_timer.timer, my_timer_func, 0);
    my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
    add_timer(&my_timer.timer);

    return 0;
}

static void __exit simple_led_exit(void)
{
    printk("ch5: Bye LED\n");
    gpio_set_value(LED1, 0);
    gpio_free(LED1);
    gpio_free(SWITCH);

    del_timer(&my_timer.timer);
}

module_init(simple_led_init);
module_exit(simple_led_exit);