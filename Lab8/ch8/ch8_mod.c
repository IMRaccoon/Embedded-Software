#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/interrupt.h>

MODULE_LICENSE("GPL");

#define SENSOR 17
#define LED 5

static int irq_num;

struct my_timer_info
{
    struct timer_list timer;
    long delay_jiffies;
};

static struct my_timer_info my_timer;

static void off_led(struct timer_list *t)
{
    unsigned long flags;

    printk("ch8 : Auto Off\n");
    local_irq_save(flags);

    gpio_set_value(LED, 0);

    local_irq_restore(flags);
}

static irqreturn_t switch_irq_isr(int irq, void *dev_id)
{
    unsigned long flags;

    printk("ch8 : Active\n");
    local_irq_save(flags);
    gpio_set_value(LED, 0);
    gpio_set_value(LED, 1);
    mod_timer(&my_timer.timer, jiffies + my_timer.delay_jiffies);

    local_irq_restore(flags);
    return IRQ_HANDLED;
}

static int __init switch_irq_init(void)
{
    int ret = 0;
    printk("ch8 : init module\n");

    my_timer.delay_jiffies = msecs_to_jiffies(2000);
    timer_setup(&my_timer.timer, off_led, 0);
    my_timer.timer.expires = jiffies + my_timer.delay_jiffies;
    add_timer(&my_timer.timer);

    gpio_request_one(SENSOR, GPIOF_IN, "sensor");
    gpio_request_one(LED, GPIOF_OUT_INIT_LOW, "LED1");

    irq_num = gpio_to_irq(SENSOR);
    ret = request_irq(irq_num, switch_irq_isr, IRQF_TRIGGER_RISING, "sensor_irq", NULL);
    if (ret)
    {
        printk("ch8 : Unable to reset request IRQ : %d\n", irq_num);
    }
    else
    {
        printk("ch8 : Enable to set request IRQ : %d\n", irq_num);
    }

    return 0;
}

static void __exit switch_irq_exit(void)
{
    printk("ch8 : exit module\n");
    del_singleshot_timer_sync(&my_timer.timer);
    disable_irq(irq_num);
    free_irq(irq_num, NULL);

    gpio_set_value(LED, 0);
    gpio_free(SENSOR);
    gpio_free(LED);
}

module_init(switch_irq_init);
module_exit(switch_irq_exit);