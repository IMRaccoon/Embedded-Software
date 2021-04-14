obj-m := ku_ipc.o

KERNELDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) modules

clean:
	$(MAKE) -C $(KERNELDIR) M=$(PWD) clean

test:
	gcc -o sender ku_ipc_sender.c
	gcc -o reader ku_ipc_reader.c