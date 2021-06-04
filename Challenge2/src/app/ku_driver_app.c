#include <stdio.h>
#include <lib/ku_sensor.c>
#include <lib/ku_actuator.c>

int main(int argc, char* argv[]) {
    int ret, dev;
    int mode;
    int distance, option;
    int duration;

    if (argc != 5) {
        printf("Insert four arguments \n");
        printf("First argument = Mode for Driver (1: Sensor, 2: Actuator)\n");
        printf("Second argument = for sensor, distance for ultra sensor (5~20). for actuator, distance for motor (5~20) \n");
        printf("Third argument = for sensor, sound pitch (0~6). for actuator, motor mode (1: accel, 2: auto)\n");
        printf("Fourth argument = Duration Option\n");
        return -1;
    }

    mode = atoi(argv[1]);
    distance = atoi(argv[2]);
    option = atoi(argv[3]);
    duration = atoi(argv[4]);

    if (mode == 1) {
        ret = ku_sens_init(distance, option);
        if (ret == -1) {
            printf("Sensor is already in use\n");
            return 0;
        }
        else if (ret == -2) {
            printf("Sensor arguments are incorrect\n");
            return 0;
        }

        ret = ku_sens_start(duration);
        if (ret == -1) {
            printf("Sensor can't run\n");
            return 0;
        }
        else if (ret == -2) {
            printf("Sensor arguments are incorrect\n");
            return 0;
        }
    }
    else if (mode == 2) {
        ret = ku_act_init(distance, option);
        if (ret == -1) {
            printf("Actuator is already in use\n");
            return 0;
        }
        else if (ret == -2) {
            printf("Actuator arguments are incorrect\n");
            return 0;
        }

        ret = ku_act_start(duration);
        if (ret == -1) {
            printf("Actuator can't run\n");
            return 0;
        }
        else if (ret == -2) {
            printf("Actuator arguments are incorrect\n");
            return 0;
        }
    }

    return 0;
}