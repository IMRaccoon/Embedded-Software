#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include "simple_param.h"

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