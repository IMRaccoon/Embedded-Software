#define DEV_NAME "simple_param_dev"

#define IOCTL_START_NUM1 1
#define IOCTL_START_NUM2 2

#define PARAM_IOCTL_NUM 'z'

#define PARAM_GET _IOWR(PARAM_IOCTL_NUM, IOCTL_START_NUM1, unsigned long)
#define PARAM_SET _IOWR(PARAM_IOCTL_NUM, IOCTL_START_NUM2, unsigned long)