obj-m += lag.o

all: 
	make -C /lib/modules/`uname -r`/build M=`pwd`
clean:
	rm lag.ko
