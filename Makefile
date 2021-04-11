TARGET = litefurym

KDIR = /lib/modules/$(shell uname -r)/build

obj-m += $(TARGET).o
$(TARGET)-objs := lfmod.o lfmod_ioctl.o lfmod_file.o

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean
