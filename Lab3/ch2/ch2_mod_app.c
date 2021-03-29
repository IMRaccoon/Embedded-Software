#include <stdio.h>
#include <sys/fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define IOCTL_START_NUM 1
#define IOCTL_NUM_GET IOCTL_START_NUM + 1
#define IOCTL_NUM_SET IOCTL_START_NUM + 2
#define IOCTL_NUM_ADD IOCTL_START_NUM + 3
#define IOCTL_NUM_MUL IOCTL_START_NUM + 4

#define CH_IOCTL_MAGIC 'z'
#define GET_IOCTL _IOR(CH_IOCTL_MAGIC, IOCTL_NUM_GET, long)
#define SET_IOCTL _IOWR(CH_IOCTL_MAGIC, IOCTL_NUM_SET, long)
#define ADD_IOCTL _IOWR(CH_IOCTL_MAGIC, IOCTL_NUM_ADD, long)
#define MUL_IOCTL _IOWR(CH_IOCTL_MAGIC, IOCTL_NUM_MUL, long)

void main(void)
{
    int dev;
    long result = 200;

    dev = open("/dev/ch2_dev", O_RDWR);

    ioctl(dev, SET_IOCTL, 200);
    result = ioctl(dev, GET_IOCTL, NULL);
    printf("%ld\n", result);
    ioctl(dev, ADD_IOCTL, 300);
    result = ioctl(dev, GET_IOCTL, NULL);
    printf("%ld\n", result);
    ioctl(dev, MUL_IOCTL, 100);
    result = ioctl(dev, GET_IOCTL, NULL);
    printf("%ld\n", result);

    close(dev);
}