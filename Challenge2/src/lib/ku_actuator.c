#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "ku_lib.h"


int main(int argc, char* argv[]) {
    int dev, ret;


    dev = open("/dev/ku_driver", O_RDWR);
    ret = ioctl(dev, IOCTL_ACTUATOR_INIT, NULL);
    if (ret == -1) {
        printf("Actuator Init Error\n");
        return 0;
    }

    ret = ioctl(dev, IOCTL_ACTUATOR_START, NULL);
    if (ret == -1) {
        printf("Actuator Start Error\n");
        return 0;
    }

    sleep(10);

    ioctl(dev, IOCTL_ACTUATOR_END, NULL);
    close(dev);
    return 0;
}