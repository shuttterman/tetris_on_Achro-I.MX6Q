obj-m = dot_matrix_driver.o push_switch_driver.o led_driver.o sseg_driver.o

PWD = $(shell pwd)

all: driver app

driver:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

app:
	gcc -Wall -static -o tetris tetris.c -lpthread

clean:
	rm -rf *.ko *.mod.* *.o tetris Module.symvers modules.order .dot_matrix* .tmp* .sseg* .led* .push_switch*
