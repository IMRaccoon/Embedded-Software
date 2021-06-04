obj-m := ku_driver.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
DRIVER := $(shell pwd)/driver
APP := $(shell pwd)/app/ku_driver_app
SENSOR := $(shell pwd)/lib/ku_sensor
ACTUATOR := $(shell pwd)/lib/ku_actuator

default:
	$(MAKE) -C $(KDIR) M=$(DRIVER) $(ARM) modules
	arm-linux-gnueabi-gcc ${SENSOR}.c -o ${SENSOR} -I$(shell pwd)
	arm-linux-gnueabi-gcc ${ACTUATOR}.c -o ${ACTUATOR} -I$(shell pwd)

clean:
	$(MAKE) -C $(KDIR) M=$(DRIVER) clean
	rm ${APP} ${ACTUATOR} ${SENSOR}
scp:
	scp ${DRIVER}/$(obj-m:.o=.ko) mknod.sh ${APP} ${SENSOR} ${ACTUATOR} pi@10.10.10.12:~/
