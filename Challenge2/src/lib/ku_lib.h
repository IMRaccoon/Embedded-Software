#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM + 1
#define IOCTL_NUM2 IOCTL_START_NUM + 2
#define IOCTL_NUM3 IOCTL_START_NUM + 3
#define IOCTL_NUM4 IOCTL_START_NUM + 4
#define IOCTL_NUM5 IOCTL_START_NUM + 5
#define IOCTL_NUM6 IOCTL_START_NUM + 6

#define SIMPLE_IOCTL_NUM 'z'
#define IOCTL_SENSOR_INIT     _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define IOCTL_SENSOR_START    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define IOCTL_SENSOR_END      _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define IOCTL_ACTUATOR_INIT   _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM4, unsigned long *)
#define IOCTL_ACTUATOR_START  _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM5, unsigned long *)
#define IOCTL_ACTUATOR_END    _IOWR(SIMPLE_IOCTL_NUM, IOCTL_NUM6, unsigned long *)

struct ku_sensor_data {
    int distance;
    int sound_option;
};

struct ku_actuator_data {
    int distance;
    int mode;
};