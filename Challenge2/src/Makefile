obj-m := ku_driver.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
DRIVER_PWD := $(shell pwd)/driver
APP_PWD := $(shell pwd)/app

default:
	$(MAKE) -C $(KDIR) M=$(DRIVER_PWD) $(ARM) modules
	arm-linux-gnueabi-gcc ./app/$(obj-m:.o=_app.c) -o ./app/$(obj-m:.o=_app)
clean:
	$(MAKE) -C $(KDIR) M=$(DRIVER_PWD) clean
	rm ./app/$(obj-m:.o=_app)
scp:
	scp ${DRIVER_PWD}/$(obj-m:.o=.ko) mknod.sh ${APP_PWD}/$(obj-m:.o=_app) pi@10.10.10.12:~/
