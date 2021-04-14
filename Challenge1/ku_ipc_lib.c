#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "ku_ipc.h"

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

struct ku_msg_snd_buf
{
    long type;
    int size;
    char str[128];
};

struct ku_msg_rcv_buf
{
    long type;
    char str[128];
};

struct ku_msg_snd_data
{
    int qid;
    struct ku_msg_snd_buf *data;
};

struct ku_msg_rcv_data
{
    int qid;
    int size;
    struct ku_msg_rcv_buf *data;
};

int fd = 0;

int checkQueueID(int msqid)
{
    if (msqid < 0 || msqid > 9)
    {
        return 1;
    }
    return 0;
}

// Message Queue Open
int ku_msgget(int key, int msgflg)
{
    int ret;
    if (checkQueueID(key) || !((msgflg & KU_IPC_CREATE) || (msgflg & KU_IPC_EXCL)))
    {
        return -1;
    }
    else if (!!fd)
    {
        return -1;
    }

    fd = open("/dev/ku_ipc", O_RDWR);
    if ((msgflg & KU_IPC_CREATE) != 0)
    {
        ret = ioctl(fd, MESSAGE_GET_CREATE, key);
        printf("open %d\n", ret);
    }
    else if ((msgflg & KU_IPC_EXCL) != 0)
    {
        ret = ioctl(fd, MESSAGE_GET_EXCL, key);
    }
    else
    {
        fd = 0;
        return -1;
    }

    if (ret == -1)
    {
        fd = 0;
        return ret;
    }
    return ret;
}

int ku_msgclose(int msqid)
{
    if (checkQueueID(msqid) || !fd)
    {
        return -1;
    }

    return ioctl(fd, MESSAGE_CLOSE, msqid);
}

// Message Send
int ku_msgsnd(int msqid, void *msgp, int msgsz, int msgflg)
{
    if (checkQueueID(msqid) || !((msgflg & KU_IPC_NOWAIT) || msgflg == 0) || !fd)
    {
        return -1;
    }
    else if (msgsz < 0 || msgsz > 128)
    {
        return -1;
    }

    struct ku_msg_rcv_buf *tmp = msgp;
    struct ku_msg_snd_buf data =
        {
            .size = msgsz,
            .type = tmp->type,
        };

    memcpy(data.str, tmp->str, msgsz);

    struct ku_msg_snd_data send_data =
        {
            .qid = msqid,
            .data = &data};

    if ((msgflg & KU_IPC_NOWAIT) != 0)
    {
        return ioctl(fd, MESSAGE_SEND_NOWAIT, &send_data);
    }
    else if (msgflg == 0)
    {
        return ioctl(fd, MESSAGE_SEND_WAIT, &send_data);
    }
    return -1;
}

// Message Receive
int ku_msgrcv(int msqid, void *msgp, int msgsz, long msgtyp, int msgflg)
{
    if (checkQueueID(msqid) || !((msgflg & KU_IPC_NOWAIT) || (msgflg & KU_MSG_NOERROR)) || !fd)
    {
        return -1;
    }
    else if (msgsz < 0 || msgsz > 128)
    {
        return -1;
    }

    struct ku_msg_rcv_buf *tmp = msgp;
    tmp->type = msgtyp;

    struct ku_msg_rcv_data send_data =
        {
            .qid = msqid,
            .size = msgsz,
            .data = tmp};

    if ((msgflg & KU_IPC_NOWAIT) != 0 && (msgflg & KU_MSG_NOERROR) != 0)
    {
        return ioctl(fd, MESSAGE_RECEIVE_NOWAIT_NOERROR, &send_data);
    }
    else if ((msgflg & KU_IPC_NOWAIT) != 0)
    {
        return ioctl(fd, MESSAGE_RECEIVE_NOWAIT_ERROR, &send_data);
    }
    else if ((msgflg & KU_MSG_NOERROR) != 0)
    {
        return ioctl(fd, MESSAGE_RECEIVE_WAIT_NOERROR, &send_data);
    }
    return ioctl(fd, MESSAGE_RECEIVE_WAIT_ERROR, &send_data);
}
