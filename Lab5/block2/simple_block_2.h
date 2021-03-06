#define IOCTL_START_NUM 0x00
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3
#define IOCTL_NUM4 IOCTL_START_NUM + 4

#define SIMPLE_IOCTL_NUM 0xA4
#define WQ _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long)
#define WQ_EX _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long)
#define WQ_WAKE_UP _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long)
#define WQ_WAKE_UP_ALL _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long)