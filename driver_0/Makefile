
obj-m :=pci_driver_hessen.o                  #产生hello模块的目标文件
all:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) modules    #编译模块
clean:
	make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean      #清理
