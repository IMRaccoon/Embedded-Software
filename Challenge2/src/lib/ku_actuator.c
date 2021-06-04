#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "ku_lib.h"

int fd = 0;

// mode = 1 ACCEL, mode = 2 AUTO
int ku_act_init(int distance, int mode) {
    int ret;
    struct ku_actuator_data send;

    if (!!fd) {
        return -1;
    }
    if (mode != 1 && mode != 2) {
        return -2;
    }
    if (distance < 5 || distance > 20) {
        return -2;
    }

    send.distance = distance;
    send.mode = mode;

    fd = open("/dev/ku_driver", O_RDWR);
    ret = ioctl(fd, IOCTL_ACTUATOR_INIT, &send);

    return ret;
}

int ku_act_start(int run_time) {
    int ret;

    if (!fd) {
        return -1;
    }

    ret = ioctl(fd, IOCTL_ACTUATOR_START, NULL);
    if (ret == -1) {
        return -1;
    }

    sleep(run_time);

    ioctl(fd, IOCTL_ACTUATOR_END, NULL);
    close(fd);
    return 0;
}

int main(void) {
    int ret = 0;

    ret = ku_act_init(15, 2);
    if (ret != 0) {
        printf("Actuator Init Error %d\n", ret);
        return 0;
    }
    ret = ku_act_start(10);
    if (ret != 0) {
        printf("Actuator Start Error\n");
        return 0;
    }
}