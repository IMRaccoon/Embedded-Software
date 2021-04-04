#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/list.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/uaccess.h>

#include "ch3.h"
#define DEV_NAME "ch3_dev"

MODULE_LICENSE("GPL");

struct msg_list
{
    struct list_head list;
    struct msg_st *msg;
};

static struct msg_list msg_list_head;
struct msg_st *kern_msg;
struct msg_st msg_init = {0, {'\0'}};

spinlock_t my_lock;

/**
 *  List가 비어있는 경우, len = 0, str = {'/0'};
 *  List가 있는 경우, 가장 앞에 있는 애의 데이터를 복사하고
 *  해당 노드를 삭제한다.
 **/
static int ch3_read(struct msg_st *buf)
{
    int ret = 0;
    struct msg_list *tmp = 0;

    spin_lock(&my_lock);
    if (list_empty(&msg_list_head.list) == 1)
    {
        ret = copy_to_user(buf, &msg_init, sizeof(struct msg_st));
    }
    else
    {
        tmp = list_first_entry(&msg_list_head.list, struct msg_list, list);
        ret = copy_to_user(buf, tmp->msg, sizeof(struct msg_st));
        list_del(&tmp->list);
    }
    spin_unlock(&my_lock);

    return ret;
}

/**
 *  List에 node를 추가하는데 해당 노드는 msg_st 타입이다.
 *  먼저 새로운 list 노드를 메모리 할당을 해준다.
 *  이 후, list 내부의 msg에 들어온 buf의 내용을 복사해준다.
 *  복사 한 뒤, 기존 list 마지막에 넣어준다.
 **/
static int ch3_write(struct msg_st *buf)
{
    struct msg_list *new_item;
    int ret = 0;

    spin_lock(&my_lock);
    new_item = (struct msg_list *)kmalloc(sizeof(struct msg_list), GFP_KERNEL);
    new_item->msg = (struct msg_st *)vmalloc(sizeof(struct msg_st));
    ret = copy_from_user(new_item->msg, buf, sizeof(struct msg_st));
    list_add_tail(&new_item->list, &msg_list_head.list);
    spin_unlock(&my_lock);

    return ret;
}

static long ch3_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    struct msg_st *user_buf;
    int ret = 0;

    user_buf = (struct msg_st *)arg;

    switch (cmd)
    {
    case CH3_IOCTL_READ:
        ret = ch3_read(user_buf);
        break;
    case CH3_IOCTL_WRITE:
        ret = ch3_write(user_buf);
        break;
    }
    return ret;
}

static int ch3_open(struct inode *inode, struct file *file)
{
    return 0;
}

static int ch3_release(struct inode *inode, struct file *file)
{
    return 0;
}

struct file_operations ch3_fops = {
    .unlocked_ioctl = ch3_ioctl,
    .open = ch3_open,
    .release = ch3_release};

static dev_t dev_num;
static struct cdev *cd_cdev;

/**
 * 리스트를 초기화해준다.
 * 전역 변수인 msg_st 구조체의 kern_msg 를 초기화 해준다.
 **/
static int __init ch3_init(void)
{
    int ret;
    INIT_LIST_HEAD(&msg_list_head.list);

    kern_msg = (struct msg_st *)vmalloc(sizeof(struct msg_st));
    memset(kern_msg, '\0', sizeof(struct msg_st));

    alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
    cd_cdev = cdev_alloc();
    cdev_init(cd_cdev, &ch3_fops);
    ret = cdev_add(cd_cdev, dev_num, 1);
    if (ret < 0)
    {
        return -1;
    }

    return 0;
}

/**
 *  리스트 내부의 node들의 메모리 할당을 해제해준다.
 **/
static void __exit ch3_exit(void)
{
    struct msg_list *tmp = 0;
    struct list_head *pos = 0;
    struct list_head *q = 0;

    list_for_each_safe(pos, q, &msg_list_head.list)
    {
        tmp = list_entry(pos, struct msg_list, list);
        list_del(pos);
        vfree(tmp->msg);
        kfree(tmp);
    }

    cdev_del(cd_cdev);
    unregister_chrdev_region(dev_num, 1);
    vfree(kern_msg);
}

module_init(ch3_init);
module_exit(ch3_exit);