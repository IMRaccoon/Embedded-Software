obj-m := ku_driver.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
DRIVER := $(shell pwd)/driver
APP_PWD := $(shell pwd)/app
SENSOR := $(shell pwd)/lib/ku_sensor
ACTUATOR := $(shell pwd)/lib/ku_actuator

default:
	$(MAKE) -C $(KDIR) M=$(DRIVER) $(ARM) modules
	arm-linux-gnueabi-gcc ${SENSOR}.c -o ${SENSOR}
	arm-linux-gnueabi-gcc ${ACTUATOR}.c -o ${ACTUATOR}
clean:
	$(MAKE) -C $(KDIR) M=$(DRIVER) clean
	rm ${SENSOR} ${ACTUATOR}
scp:
	scp ${DRIVER}/$(obj-m:.o=.ko) mknod.sh ${SENSOR} ${ACTUATOR} pi@10.10.10.12:~/
