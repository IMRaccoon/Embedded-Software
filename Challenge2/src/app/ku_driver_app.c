#include <stdio.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <unistd.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3
#define IOCTL_NUM4 IOCTL_START_NUM + 4

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_SENSOR_START    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_SENSOR_END      _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define IOCTL_ACTUATOR_START  _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define IOCTL_ACTUATOR_END    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

int main(int argc, char* argv[]) {
    int dev;


    dev = open("/dev/ku_driver", O_RDWR);
    ioctl(dev, IOCTL_SENSOR_START, NULL);

    sleep(10);

    ioctl(dev, IOCTL_SENSOR_END, NULL);
    close(dev);
    return 0;
}