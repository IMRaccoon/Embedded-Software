#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "ku_lib.h"


int main(int argc, char* argv[]) {
    int dev;


    dev = open("/dev/ku_driver", O_RDWR);
    ioctl(dev, IOCTL_ACTUATOR_START, NULL);

    sleep(10);

    ioctl(dev, IOCTL_ACTUATOR_END, NULL);
    close(dev);
    return 0;
}