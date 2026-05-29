	obj-m += helloKernel.o
	obj-m += second.o
	obj-m += kernsum.o
	obj-m += elev.o

PWD := $(CURDIR)

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
