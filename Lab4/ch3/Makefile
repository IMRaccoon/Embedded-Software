obj-m := ch3_mod.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules
	gcc -o $(obj-m:.o=_app) $(obj-m:.o=_app.c)

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean
	rm $(obj-m:.o=_app)
	