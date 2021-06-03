obj-m := ku_driver.o

KDIR := ~/linux-rpi/
ARM := ARCH=arm CROSS_COMPILE=/usr/bin/arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc ../app/$(obj-m:.o=_app.c) -o ../app/$(obj-m:.o=_app)
clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm ../app/$(obj-m:.o=_app)
scp:
	scp $(obj-m:.o=.ko) mknod.sh ../app/$(obj-m:.o=_app) pi@10.10.10.12:~/
