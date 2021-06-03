#include <linux/init.h>
#include <linux/uaccess.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/timer.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/rculist.h>
#include <asm/delay.h>

#define DEV_NAME "ku_driver"

#define SPEAKER     12
#define ULTRA_TRIG  17
#define ULTRA_ECHO  18

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3
#define IOCTL_NUM4 IOCTL_START_NUM + 4

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_SENSOR_START    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_SENSOR_END      _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define IOCTL_ACTUATOR_START  _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define IOCTL_ACTUATOR_END    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

#define SPEAKER_SOUND   1275

MODULE_LICENSE("GPL");

static int irq_num;
static int echo_valid_flag = 3;

static ktime_t echo_start;
static ktime_t echo_stop;

struct tasklet_struct sound_tasklet;

struct sensor_timer_t
{
    struct timer_list timer;
    long delay_jiffies;
    int data;
};
static struct sensor_timer_t sensor_timer;

spinlock_t distance_lock;
unsigned long __rcu* distance;

static dev_t dev_num;
static struct cdev* cd_cdev;

// Interrupt Handler (Bottom Half)
static void sensor_timer_func(struct timer_list* t)
{
    if (echo_valid_flag == 3) {
        echo_start = ktime_set(0, 1);
        echo_stop = ktime_set(0, 1);
        echo_valid_flag = 0;

        gpio_set_value(ULTRA_TRIG, 1);
        udelay(10);
        gpio_set_value(ULTRA_TRIG, 0);

        echo_valid_flag = 1;
    }
}

void speaker_play(unsigned long data) {
    int i = 0;
    unsigned long* old_distance;

    rcu_read_lock();
    old_distance = rcu_dereference(distance);
    rcu_read_unlock();
    printk("simple_ultra: Detect %ld cm \n", *old_distance);

    if (*old_distance != 0 && *old_distance <= 20) {
        for (i = 0; i < 100; i++) {
            gpio_set_value(SPEAKER, 1);
            udelay(SPEAKER_SOUND);
            gpio_set_value(SPEAKER, 0);
            udelay(SPEAKER_SOUND);
        }
        if (*old_distance < 5) {
            mdelay(10);
        }
        else if (*old_distance < 10) {
            mdelay(0);
        }
        else {
            mdelay(100);
        }
        tasklet_schedule(&sound_tasklet);
    }
}

static irqreturn_t ultra_isr(int irq, void* dev_id) {
    ktime_t tmp_time;
    s64 time;
    unsigned long flags;
    unsigned long* new, * old;

    tmp_time = ktime_get();
    if (echo_valid_flag == 1) {
        // kthread kill
        if (gpio_get_value(ULTRA_ECHO) == 1) {
            echo_start = tmp_time;
            echo_valid_flag = 2;
        }
    }
    else if (echo_valid_flag == 2) {
        if (gpio_get_value(ULTRA_ECHO) == 0) {
            echo_stop = tmp_time;
            time = ktime_to_us(ktime_sub(echo_stop, echo_start));

            spin_lock_irqsave(&distance_lock, flags);
            new = (unsigned long*)kmalloc(sizeof(unsigned long), GFP_KERNEL);
            *new = (int)time / 58;
            old = rcu_dereference(distance);
            rcu_assign_pointer(distance, new);
            // synchronize_rcu();

            kfree(old);
            spin_unlock_irqrestore(&distance_lock, flags);

            printk("simple_ultra: Detect %ld cm \n", *new);
            echo_valid_flag = 3;
            // kthread start
            tasklet_schedule(&sound_tasklet);
        }
    }
    mod_timer(&sensor_timer.timer, jiffies + sensor_timer.delay_jiffies);

    return IRQ_HANDLED;
}

static long ku_driver_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {
    switch (cmd) {
    case IOCTL_SENSOR_START:
        // Timer Boot
        sensor_timer.delay_jiffies = msecs_to_jiffies(1000);
        timer_setup(&sensor_timer.timer, sensor_timer_func, 0);
        sensor_timer.timer.expires = jiffies + sensor_timer.delay_jiffies;
        add_timer(&sensor_timer.timer);
        break;
    case IOCTL_SENSOR_END:
        tasklet_kill(&sound_tasklet);
        del_singleshot_timer_sync(&sensor_timer.timer);
        break;
    }
    return 0;
}

static int ku_driver_open(struct inode* inode, struct file* file) {
    return 0;
}

static int ku_driver_release(struct inode* inode, struct file* file) {
    return 0;
}

struct file_operations ku_driver_fops = {
    .unlocked_ioctl = ku_driver_ioctl,
    .open = ku_driver_open,
    .release = ku_driver_release,
};


static int __init ku_driver_init(void) {
    int ret;
    unsigned long* data;

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ku_driver_fops);
    cdev_add(cd_cdev, dev_num, 1);
    printk("ku_driver: Init Module\n");

    spin_lock_init(&distance_lock);
    data = (unsigned long*)kmalloc(sizeof(unsigned long), GFP_KERNEL);
    *data = 0;
    rcu_assign_pointer(distance, data);

    tasklet_init(&sound_tasklet, speaker_play, *data);

    gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");
    gpio_request_one(ULTRA_TRIG, GPIOF_OUT_INIT_LOW, "ULTRA_TRIG");
    gpio_request_one(ULTRA_ECHO, GPIOF_IN, "ULTRA_ECHO");

    irq_num = gpio_to_irq(ULTRA_ECHO);
    ret = request_irq(irq_num, ultra_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ULTRA_ECHO", NULL);
    if (ret) {
        printk("ku_driver: Unable to request IRQ: %d\n", ret);
        free_irq(irq_num, NULL);
    }

    return 0;
}

static void __exit ku_driver_exit(void) {
    unsigned long flags;
    unsigned long* data;

    spin_lock_irqsave(&distance_lock, flags);
    data = rcu_dereference(distance);
    rcu_assign_pointer(distance, NULL);
    kfree(data);
    spin_unlock_irqrestore(&distance_lock, flags);

    gpio_set_value(SPEAKER, 0);

    free_irq(irq_num, NULL);
    gpio_free(SPEAKER);
    gpio_free(ULTRA_TRIG);
    gpio_free(ULTRA_ECHO);

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk("ku_driver: Exit Module\n");
}

module_init(ku_driver_init);
module_exit(ku_driver_exit);