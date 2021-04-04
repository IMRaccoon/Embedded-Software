#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include "ch3.h"

int main(void)
{
    int dev;
    struct msg_st send_msg[2] = {{0, "First Value"}, {1, "Second Value"}};
    struct msg_st receive_msg;
    send_msg[0].len = strlen(send_msg[0].str);
    send_msg[1].len = strlen(send_msg[1].str);

    dev = open("/dev/ch3_dev", O_RDWR);
    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    ioctl(dev, CH3_IOCTL_WRITE, &send_msg[0]);

    printf("ch3 send: size %d, content: %s\n", send_msg[0].len, send_msg[0].str);

    ioctl(dev, CH3_IOCTL_WRITE, &send_msg[1]);

    printf("ch3 send: size %d, content: %s\n", send_msg[1].len, send_msg[1].str);

    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    ioctl(dev, CH3_IOCTL_WRITE, &send_msg[0]);

    printf("ch3 send: size %d, content: %s\n", send_msg[0].len, send_msg[0].str);

    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    ioctl(dev, CH3_IOCTL_READ, &receive_msg);

    printf("ch3 receive, size %d, content: %s\n", receive_msg.len, receive_msg.str);

    close(dev);
    return 0;
}