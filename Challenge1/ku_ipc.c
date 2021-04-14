#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <linux/list.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/uaccess.h>
#include "ku_ipc.h"

#define DEV_NAME "ku_ipc"
#define MAX_QUEUE_LENGTH 10
#define MAX_CONCURRENT_PROCESS 3

#define IOCTL_START_NUM 0x00
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3
#define IOCTL_NUM4 IOCTL_START_NUM + 4
#define IOCTL_NUM5 IOCTL_START_NUM + 5
#define IOCTL_NUM6 IOCTL_START_NUM + 6
#define IOCTL_NUM7 IOCTL_START_NUM + 7
#define IOCTL_NUM8 IOCTL_START_NUM + 8
#define IOCTL_NUM9 IOCTL_START_NUM + 9

#define SIMPLE_IOCTL_NUM 0xA4
#define MESSAGE_GET_CREATE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define MESSAGE_GET_EXCL _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long)
#define MESSAGE_CLOSE _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long)
#define MESSAGE_SEND_WAIT _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long)
#define MESSAGE_SEND_NOWAIT _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long)
#define MESSAGE_RECEIVE_WAIT_ERROR _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM6, unsigned long)
#define MESSAGE_RECEIVE_NOWAIT_ERROR _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM7, unsigned long)
#define MESSAGE_RECEIVE_WAIT_NOERROR _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM8, unsigned long)
#define MESSAGE_RECEIVE_NOWAIT_NOERROR _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM9, unsigned long)

#define TRUE 1
#define FALSE 0

struct ku_msg_list
{
    struct list_head list_head;
    struct ku_msg_snd_buf *data;
};

struct ku_msg_queue
{
    struct ku_msg_list ku_queue;
    pid_t using_pid[MAX_CONCURRENT_PROCESS];
    int queue_count;
    int queue_vol;
    int pid_index;
};

struct ku_msg_snd_buf
{
    long type;
    int size;
    char str[128];
};

struct ku_msg_snd_data
{
    int qid;
    int size;
    struct ku_msg_snd_buf *data;
};

struct ku_msg_rcv_buf
{
    long type;
    char str[128];
};

struct ku_msg_rcv_data
{
    int qid;
    int size;
    struct ku_msg_rcv_buf *data;
};

static dev_t dev_num;
static struct cdev *cd_cdev;
spinlock_t ku_ipc_lock;
wait_queue_head_t ku_ipc_snd_wq[10];
wait_queue_head_t ku_ipc_rcv_wq[10];

static struct ku_msg_queue msg_queue_list[MAX_QUEUE_LENGTH];
struct ku_msg_queue *cur_queue;

MODULE_LICENSE("DUAL BSD/GPL");

int check_pid(struct ku_msg_queue *cur)
{
    int i;
    for (i = 0; i < cur->pid_index; i++)
    {
        if (cur->using_pid[i] == current->pid)
        {
            return 0;
        }
    }
    return 1;
}

int message_get_create(int qid)
{
    cur_queue = &msg_queue_list[qid];
    printk("ku_ipc: current pid %d\n", cur_queue->pid_index);
    if (cur_queue->pid_index == MAX_CONCURRENT_PROCESS)
    {
        return -1;
    }
    else if (!check_pid(cur_queue))
    {
        return -1;
    }
    cur_queue->using_pid[cur_queue->pid_index++] = current->pid;
    return qid;
}

int message_get_excl(int qid)
{
    cur_queue = &msg_queue_list[qid];

    if (cur_queue->pid_index == MAX_CONCURRENT_PROCESS)
    {
        return -1;
    }
    else if (cur_queue->pid_index != 0)
    {
        return -1;
    }

    cur_queue->using_pid[cur_queue->pid_index++] = current->pid;
    return qid;
}

int message_close(int qid)
{
    int flag = -1;
    int i;
    cur_queue = &msg_queue_list[qid];
    for (i = 0; i < cur_queue->pid_index; i++)
    {
        if (cur_queue->using_pid[i] == current->pid)
        {
            flag = i;
        }
    }

    if (flag == -1)
    {
        return -1;
    }

    for (i = flag; i < cur_queue->pid_index - 1; i++)
    {
        cur_queue->using_pid[i] = cur_queue->using_pid[i + 1];
    }

    cur_queue->using_pid[cur_queue->pid_index--] = 0;
    return 0;
}

int message_send(struct ku_msg_snd_data *arg, int is_wait)
{
    int ret = 0;
    struct ku_msg_list *add_list;
    cur_queue = &msg_queue_list[arg->qid];

    if (cur_queue->queue_count > KUIPC_MAXMSG || (cur_queue->queue_vol + arg->data->size) >= KUIPC_MAXVOL)
    {
        if (is_wait == TRUE)
        {
            printk("ku_ipc: send wait count: %d, vol: %d\n", cur_queue->queue_count, cur_queue->queue_vol);
            ret = wait_event_interruptible_exclusive(ku_ipc_snd_wq[arg->qid], (cur_queue->queue_count <= KUIPC_MAXMSG) && ((cur_queue->queue_vol + arg->data->size) < KUIPC_MAXVOL));
            if (ret < 0)
            {
                message_close(arg->qid);
                return -1;
            }
        }
        else if (is_wait == FALSE)
        {
            return -1;
        }
    }
    else
    {
        spin_lock(&ku_ipc_lock);
        cur_queue->queue_vol += arg->data->size;
        cur_queue->queue_count++;
        spin_unlock(&ku_ipc_lock);
    }

    add_list = (struct ku_msg_list *)kmalloc(sizeof(struct ku_msg_list), GFP_KERNEL);
    add_list->data = (struct ku_msg_snd_buf *)vmalloc(sizeof(struct ku_msg_snd_buf));
    add_list->data->size = arg->data->size;
    add_list->data->type = arg->data->type;

    ret = copy_from_user(add_list->data->str, arg->data->str, arg->data->size);
    if (ret != 0)
    {
        vfree(add_list->data->str);
        vfree(add_list->data);
        kfree(add_list);
        return -1;
    }

    list_add_tail(&add_list->list_head, &cur_queue->ku_queue.list_head);

    spin_lock(&ku_ipc_lock);
    cur_queue->queue_vol += arg->data->size;
    cur_queue->queue_count++;
    spin_unlock(&ku_ipc_lock);
    wake_up(&ku_ipc_rcv_wq[arg->qid]);
    return ret;
}

struct ku_msg_list *find_current_list(long type)
{
    struct ku_msg_list *tmp;
    list_for_each_entry(tmp, &cur_queue->ku_queue.list_head, list_head)
    {
        if (type > 0 && tmp->data->type == type)
        {
            return tmp;
        }
        else if (type < 0 && tmp->data->type < -type)
        {
            return tmp;
        }
    }

    return NULL;
}

int message_receive(struct ku_msg_rcv_data *arg, int is_wait, int is_noerror)
{
    int ret = 0;
    int size;
    struct ku_msg_list *pos = NULL;
    cur_queue = &msg_queue_list[arg->qid];

    if (arg->data->type == 0)
    {
        pos = list_first_entry_or_null(&cur_queue->ku_queue.list_head, struct ku_msg_list, list_head);

        if (!pos)
        {
            if (is_wait == FALSE)
            {
                return -1;
            }
            printk("ku_ipc: receive wait count: %d, vol: %d\n", cur_queue->queue_count, cur_queue->queue_vol);
            ret = wait_event_interruptible(ku_ipc_rcv_wq[arg->qid], list_empty(&cur_queue->ku_queue.list_head) != 1);
            if (ret < 0)
            {
                message_close(arg->qid);
                return -1;
            }
            pos = list_first_entry_or_null(&cur_queue->ku_queue.list_head, struct ku_msg_list, list_head);
        }
    }
    else
    {
        if (is_wait == TRUE)
        {
            ret = wait_event_interruptible(ku_ipc_rcv_wq[arg->qid], (pos = find_current_list(arg->data->type)) != NULL);
            if (ret < 0)
            {
                message_close(arg->qid);
                return -1;
            }
        }
        else
        {
            pos = find_current_list(arg->data->type);
            if (!pos)
            {
                return -1;
            }
        }
    }

    if (pos->data->size > arg->size)
    {
        if (is_noerror == FALSE)
        {
            return -1;
        }
        else
        {
            size = arg->size;
            ret = copy_to_user(arg->data->str, pos->data->str, arg->size);
        }
    }
    else
    {
        size = pos->data->size;
        ret = copy_to_user(arg->data->str, pos->data->str, size - 1);
    }

    spin_lock(&ku_ipc_lock);
    cur_queue->queue_vol -= pos->data->size;
    cur_queue->queue_count--;
    spin_unlock(&ku_ipc_lock);

    wake_up(&ku_ipc_snd_wq[arg->qid]);

    list_del(&pos->list_head);
    vfree(pos->data);
    kfree(pos);
    return size;
}

static long ku_ipc_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    switch (cmd)
    {
    case MESSAGE_GET_CREATE:
        return message_get_create((int)arg);
    case MESSAGE_GET_EXCL:
        return message_get_excl((int)arg);
    case MESSAGE_CLOSE:
        return message_close((int)arg);
    case MESSAGE_SEND_WAIT:
        return message_send((struct ku_msg_snd_data *)arg, TRUE);
    case MESSAGE_SEND_NOWAIT:
        return message_send((struct ku_msg_snd_data *)arg, FALSE);
    case MESSAGE_RECEIVE_WAIT_ERROR:
        return message_receive((struct ku_msg_rcv_data *)arg, TRUE, FALSE);
    case MESSAGE_RECEIVE_NOWAIT_ERROR:
        return message_receive((struct ku_msg_rcv_data *)arg, FALSE, FALSE);
    case MESSAGE_RECEIVE_WAIT_NOERROR:
        return message_receive((struct ku_msg_rcv_data *)arg, TRUE, TRUE);
    case MESSAGE_RECEIVE_NOWAIT_NOERROR:
        return message_receive((struct ku_msg_rcv_data *)arg, FALSE, TRUE);
    default:
        return -1;
    }
}

static int ku_ipc_open(struct inode *inode, struct file *file) { return 0; }
static int ku_ipc_release(struct inode *inode, struct file *file) { return 0; }

struct file_operations ku_ipc_fops = {
    .unlocked_ioctl = ku_ipc_ioctl,
    .open = ku_ipc_open,
    .release = ku_ipc_release};

static int __init ku_ipc_init(void)
{
    int ret, i;

    for (i = 0; i < MAX_QUEUE_LENGTH; i++)
    {
        INIT_LIST_HEAD(&msg_queue_list[i].ku_queue.list_head);
        msg_queue_list[i].pid_index = 0;
        msg_queue_list[i].queue_count = 0;
        msg_queue_list[i].queue_vol = 0;
        init_waitqueue_head(&ku_ipc_rcv_wq[i]);
        init_waitqueue_head(&ku_ipc_snd_wq[i]);
    }

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ku_ipc_fops);
    ret = cdev_add(cd_cdev, dev_num, 1);

    spin_lock_init(&ku_ipc_lock);

    if (ret < 0)
    {
        printk("KU IPC: Init Failure");
        return -1;
    }
    else
    {
        printk("KU IPC: Init Success\n");
        return 0;
    }
}

static void __exit ku_ipc_exit(void)
{
    int i;
    struct list_head *pos = 0;
    struct list_head *q = 0;
    struct ku_msg_list *tmp = 0;

    printk("KU IPC: Exit\n");
    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);

    for (i = 0; i < MAX_QUEUE_LENGTH; i++)
    {
        list_for_each_safe(pos, q, &msg_queue_list[i].ku_queue.list_head)
        {
            tmp = list_entry(pos, struct ku_msg_list, list_head);
            printk("ku_ipc: remove %d %ld %s\n", tmp->data->size, tmp->data->type, tmp->data->str);
            vfree(tmp->data);
            kfree(tmp);
        }
    }
}

module_init(ku_ipc_init);
module_exit(ku_ipc_exit);