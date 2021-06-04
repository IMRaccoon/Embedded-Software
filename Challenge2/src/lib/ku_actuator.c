#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM4 IOCTL_START_NUM + 4
#define IOCTL_NUM5 IOCTL_START_NUM + 5
#define IOCTL_NUM6 IOCTL_START_NUM + 6

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_ACTUATOR_INIT   _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define IOCTL_ACTUATOR_START  _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long *)
#define IOCTL_ACTUATOR_END    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM6, unsigned long *)

struct ku_actuator_data {
    int distance;
    int mode;
};

int fd_act = 0;

// mode = 1 ACCEL, mode = 2 AUTO
int ku_act_init(int distance, int mode) {
    int ret;
    struct ku_actuator_data send;

    if (!!fd_act) {
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

    fd_act = open("/dev/ku_driver", O_RDWR);
    ret = ioctl(fd_act, IOCTL_ACTUATOR_INIT, &send);

    return ret;
}

int ku_act_start(int run_time) {
    int ret;

    if (!fd_act) {
        return -1;
    }
    else if (run_time < 0 || run_time > 100) {
        return -2;
    }

    ret = ioctl(fd_act, IOCTL_ACTUATOR_START, NULL);
    if (ret == -1) {
        return ret;
    }

    sleep(run_time);

    ioctl(fd_act, IOCTL_ACTUATOR_END, NULL);
    close(fd_act);
    return 0;
}

int main(void) {
    ku_act_init(10, 1);
    ku_act_start(10);
    return 0;
}