#include <stdio.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <string.h>

int main(void)
{
    int dev;

    dev = open("/dev/simple_sensor_dev", O_RDWR);

    sleep(100);

    close(dev);
}