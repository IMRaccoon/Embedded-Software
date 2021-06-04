#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>
#include "ku_lib.h"

int fd = 0;


// Sensor Initialize
// distance (default 0, min 5, max 20), Sound (0~6)
int ku_sens_init(int distance, int sound_option) {
    int ret;
    struct ku_sensor_data send;

    if (!!fd) {
        return -1;
    }
    if (sound_option < 0 || sound_option > 6) {
        return -2;
    }
    if (distance < 5 || distance > 20) {
        return -2;
    }

    send.distance = distance;
    send.sound_option = sound_option;

    fd = open("/dev/ku_driver", O_RDWR);
    ret = ioctl(fd, IOCTL_SENSOR_INIT, &send);
    return ret;
}

int ku_sens_start(int run_time) {
    int ret;

    if (!fd) {
        return -1;
    }

    ret = ioctl(fd, IOCTL_SENSOR_START, NULL);
    if (ret == -1) {
        return -1;
    }

    sleep(run_time);

    ioctl(fd, IOCTL_SENSOR_END, NULL);
    close(fd);
    return 0;
}

int main(void) {
    int ret = 0;

    ret = ku_sens_init(10, 4);
    if (ret != 0) {
        printf("Sensor Init Error \n");
        return 0;
    }
    ret = ku_sens_start(10);
    if (ret != 0) {
        printf("Sensor Start Error\n");
        return 0;
    }
}