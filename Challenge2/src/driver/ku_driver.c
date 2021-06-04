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
#include <linux/kthread.h>
#include <asm/delay.h>

#define DEV_NAME "ku_driver"

#define SPEAKER     12
#define ULTRA_TRIG  17
#define ULTRA_ECHO  18
#define SWITCH      21
#define MOTOR_PIN1  6
#define MOTOR_PIN2  13
#define MOTOR_PIN3  19
#define MOTOR_PIN4  26

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
#define MOTOR_STEPS     8
#define MOTOR_ROUND     512
#define MOTOR_SPEED     1000

MODULE_LICENSE("GPL");

static int ultra_irq_num, switch_irq_num;
static int echo_valid_flag = 3;

static ktime_t echo_start;
static ktime_t echo_stop;

struct task_struct* sound_kthread = NULL;
struct task_struct* motor_kthread = NULL;

struct sensor_timer_t
{
    struct timer_list timer;
    long delay_jiffies;
    int data;
};
static struct sensor_timer_t sensor_timer;

spinlock_t distance_lock;
spinlock_t switch_lock;
unsigned long __rcu* distance;
unsigned long __rcu* switch_count;

static dev_t dev_num;
static struct cdev* cd_cdev;

int blue[8] = { 1, 1, 0, 0, 0, 0, 0, 1 };
int pink[8] = { 0, 1, 1, 1, 0, 0, 0, 0 };
int yellow[8] = { 0, 0, 0, 1, 1, 1, 0, 0 };
int orange[8] = { 0, 0, 0, 0, 0, 1, 1, 1 };


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

int speaker_play(void* data) {
    int i;
    unsigned long* old;

    rcu_read_unlock();
    old = rcu_dereference(distance);
    rcu_read_unlock();

    while (!kthread_should_stop()) {
        if (*old != 0 && *old <= 20) {
            for (i = 0; i < 100; i++) {
                gpio_set_value(SPEAKER, 1);
                udelay(SPEAKER_SOUND);
                gpio_set_value(SPEAKER, 0);
                udelay(SPEAKER_SOUND);
            }
            if (*old < 5) {
                mdelay(25);
            }
            else if (*old < 10) {
                mdelay(50);
            }
            else {
                mdelay(75);
            }
        }
        else {
            msleep(500);
        }
    }
    kfree(old);
    return 0;
}

void setstep(int p1, int p2, int p3, int p4) {
    gpio_set_value(MOTOR_PIN1, p1);
    gpio_set_value(MOTOR_PIN2, p2);
    gpio_set_value(MOTOR_PIN3, p3);
    gpio_set_value(MOTOR_PIN4, p4);
}

int motor_move(void* data) {
    int j = 0;
    unsigned long* old_distance, * old_switch;

    while (!kthread_should_stop()) {
        rcu_read_lock();
        old_switch = rcu_dereference(switch_count);
        rcu_read_unlock();

        if (*old_switch % 2 == 1) {

            rcu_read_lock();
            old_distance = rcu_dereference(distance);
            rcu_read_unlock();

            if (*old_distance >= 10) {
                for (j = 0; j < MOTOR_STEPS; j++) {
                    setstep(blue[j], pink[j], yellow[j], orange[j]);
                    udelay(MOTOR_SPEED);
                }
                setstep(0, 0, 0, 0);
            }
            else {
                msleep(200);
            }
        }
    }
    return 0;
}

static irqreturn_t ultra_isr(int irq, void* dev_id) {
    ktime_t tmp_time;
    s64 time;

    tmp_time = ktime_get();
    if (echo_valid_flag == 1) {
        if (gpio_get_value(ULTRA_ECHO) == 1) {
            echo_start = tmp_time;
            echo_valid_flag = 2;
        }
    }
    else if (echo_valid_flag == 2) {
        if (gpio_get_value(ULTRA_ECHO) == 0) {
            echo_stop = tmp_time;
            time = ktime_to_us(ktime_sub(echo_stop, echo_start));

            spin_lock(&distance_lock);
            *distance = (int)time / 58;
            spin_unlock(&distance_lock);

            printk("ku_driver: Distacne %ld cm\n", *distance);
            echo_valid_flag = 3;
        }
    }
    mod_timer(&sensor_timer.timer, jiffies + sensor_timer.delay_jiffies);

    return IRQ_HANDLED;
}


static irqreturn_t switch_isr(int irq, void* dev_id) {
    spin_lock(&switch_lock);
    *switch_count += 1;
    spin_unlock(&switch_lock);
    return IRQ_HANDLED;
}



static long ku_driver_ioctl(struct file* file, unsigned int cmd, unsigned long arg) {


    switch (cmd) {
    case IOCTL_SENSOR_START:
        wake_up_process(sound_kthread);
        sensor_timer.delay_jiffies = msecs_to_jiffies(500);
        timer_setup(&sensor_timer.timer, sensor_timer_func, 0);
        sensor_timer.timer.expires = jiffies + sensor_timer.delay_jiffies;
        add_timer(&sensor_timer.timer);
        break;
    case IOCTL_SENSOR_END:
        if (sound_kthread) {
            gpio_set_value(SPEAKER, 0);
            free_irq(ultra_irq_num, NULL);
            kthread_stop(sound_kthread);
        }
        del_singleshot_timer_sync(&sensor_timer.timer);
        break;
    case IOCTL_ACTUATOR_START:

        wake_up_process(motor_kthread);
        break;
    case IOCTL_ACTUATOR_END:
        if (motor_kthread) {
            kthread_stop(motor_kthread);
        }
        free_irq(switch_irq_num, NULL);
        break;
    }
    return 0;
}

static int ku_driver_open(struct inode* inode, struct file* file) {
    int ret;
    gpio_request_one(SPEAKER, GPIOF_OUT_INIT_LOW, "SPEAKER");
    gpio_request_one(ULTRA_TRIG, GPIOF_OUT_INIT_LOW, "ULTRA_TRIG");
    gpio_request_one(ULTRA_ECHO, GPIOF_IN, "ULTRA_ECHO");
    gpio_request_one(SWITCH, GPIOF_IN, "SWITCH");

    gpio_request_one(MOTOR_PIN1, GPIOF_OUT_INIT_LOW, "MOTOR_P1");
    gpio_request_one(MOTOR_PIN2, GPIOF_OUT_INIT_LOW, "MOTOR_P2");
    gpio_request_one(MOTOR_PIN3, GPIOF_OUT_INIT_LOW, "MOTOR_P3");
    gpio_request_one(MOTOR_PIN4, GPIOF_OUT_INIT_LOW, "MOTOR_P4");

    ultra_irq_num = gpio_to_irq(ULTRA_ECHO);
    switch_irq_num = gpio_to_irq(SWITCH);
    ret = request_irq(ultra_irq_num, ultra_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "ULTRA_ECHO", NULL);
    if (ret) {
        printk("ku_driver: Unable to request IRQ: %d\n", ret);
        free_irq(ultra_irq_num, NULL);
    }
    else {
        disable_irq(ultra_irq_num);
    }

    ret = request_irq(switch_irq_num, switch_isr, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "SWITCH IRQ", NULL);
    if (ret) {
        printk("ku_driver: Unable to request IRQ: %d\n", ret);
        free_irq(switch_irq_num, NULL);
    }
    else {
        disable_irq(switch_irq_num);
    }
    return 0;
}

static int ku_driver_release(struct inode* inode, struct file* file) {
    gpio_set_value(SPEAKER, 0);
    gpio_free(SPEAKER);
    gpio_free(ULTRA_TRIG);
    gpio_free(ULTRA_ECHO);
    gpio_free(SWITCH);

    gpio_free(MOTOR_PIN1);
    gpio_free(MOTOR_PIN2);
    gpio_free(MOTOR_PIN3);
    gpio_free(MOTOR_PIN4);
    return 0;
}

struct file_operations ku_driver_fops = {
    .unlocked_ioctl = ku_driver_ioctl,
    .open = ku_driver_open,
    .release = ku_driver_release,
};


static int __init ku_driver_init(void) {
    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ku_driver_fops);
    cdev_add(cd_cdev, dev_num, 1);
    printk("ku_driver: Init Module\n");

    spin_lock_init(&distance_lock);
    spin_lock_init(&switch_lock);
    distance = (unsigned long*)kmalloc(sizeof(unsigned long), GFP_KERNEL);
    *distance = 0;

    switch_count = (unsigned long*)kmalloc(sizeof(unsigned long), GFP_KERNEL);
    *switch_count = 0;

    sound_kthread = kthread_create(speaker_play, NULL, "Sound Kthread");
    if (IS_ERR(sound_kthread)) {
        sound_kthread = NULL;
        printk("ku_driver : my kernel thread ERROR \n");
    }

    motor_kthread = kthread_create(motor_move, NULL, "Motor Kthread");
    if (IS_ERR(motor_kthread)) {
        motor_kthread = NULL;
        printk("ku_driver : my kernel thread ERROR \n");
    }

    return 0;
}

static void __exit ku_driver_exit(void) {
    unsigned long flags;

    spin_lock_irqsave(&distance_lock, flags);
    kfree(distance);
    spin_unlock_irqrestore(&distance_lock, flags);
    spin_lock_irqsave(&switch_lock, flags);
    kfree(switch_count);
    spin_unlock_irqrestore(&switch_lock, flags);

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    printk("ku_driver: Exit Module\n");
}

module_init(ku_driver_init);
module_exit(ku_driver_exit);