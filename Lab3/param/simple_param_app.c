#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
// #include "simple_param.h"

#define DEV_NAME "simple_param_dev"

#define IOCTL_START_NUM1 1
#define IOCTL_START_NUM2 2

#define PARAM_IOCTL_NUM 'z'

#define PARAM_GET _IOWR(PARAM_IOCTL_NUM, IOCTL_START_NUM1, unsigned long)
#define PARAM_SET _IOWR(PARAM_IOCTL_NUM, IOCTL_START_NUM2, unsigned long)

void main(void)
{
    int dev;
    long my_id;

    dev = open("/dev/simple_param_dev", O_RDWR);

    my_id = 201511292;
    ioctl(dev, PARAM_SET, (unsigned long)my_id);

    my_id = ioctl(dev, PARAM_GET, NULL);
    printf("My id is %ld \n", my_id);

    close(dev);
}