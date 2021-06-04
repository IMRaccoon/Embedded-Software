#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_SENSOR_INIT     _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_SENSOR_START    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define IOCTL_SENSOR_END      _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)

struct ku_sensor_data {
    int distance;
    int sound_option;
};

int fd_sen = 0;


// Sensor Initialize
// distance (default 0, min 5, max 20), Sound (0~6)
int ku_sens_init(int distance, int sound_option) {
    int ret;
    struct ku_sensor_data send;

    if (!!fd_sen) {
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

    fd_sen = open("/dev/ku_driver", O_RDWR);
    ret = ioctl(fd_sen, IOCTL_SENSOR_INIT, &send);
    return ret;
}

int ku_sens_start(int run_time) {
    int ret;

    if (!fd_sen) {
        return -1;
    }
    else if (run_time < 0 || run_time > 100) {
        return -2;
    }

    ret = ioctl(fd_sen, IOCTL_SENSOR_START, NULL);
    if (ret == -1) {
        return -1;
    }

    sleep(run_time);

    ioctl(fd_sen, IOCTL_SENSOR_END, NULL);
    close(fd_sen);
    return 0;
}

int main(void) {
    ku_sens_init(10, 2);
    ku_sens_start(10);
}