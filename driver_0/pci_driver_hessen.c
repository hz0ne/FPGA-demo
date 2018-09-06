 ///
 /// @file    pci_driver_hessen.c
 /// @author  hessen(dhxsy1994@gmail.com)
 /// @date    2018-09-01 20:25:47
 ///
 //
 //
#include <linux/kernel.h> /*Needed by all modules*/
#include <linux/module.h> /*Needed for KERN_* */
#include <linux/init.h> /* Needed for the macros */
#include <linux/mod_devicetable.h>
#include <linux/pci.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/interrupt.h>
#include <linux/cdev.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <linux/ioport.h>



MODULE_LICENSE("GPL");
#define DEMO_MODULE_NAME "pci_hessen"

#define PCI_VENDOR_ID_DEMO 0x10ec
#define PCI_DEVICE_ID_DEMO 0x8168
//用于io的地址相关变量
unsigned long bar0_io_base;
unsigned long bar0_io_length;
unsigned long bar0_io_flags;
unsigned long bar0_io_end;

unsigned long bar1_io_base;
unsigned long bar1_io_length;
unsigned long bar1_io_flags;
unsigned long bar1_io_end;

void * vir_addr;


/* 指明该驱动程序适用于which PCI设备 */
static struct pci_device_id demo_pci_tbl[] ={
    {PCI_DEVICE(PCI_VENDOR_ID_DEMO,PCI_DEVICE_ID_DEMO),},
    {0,},
};

/*
struct pci_dev {
该结构体在内核有定义
};
*/



static int pci_info_print(struct pci_dev *pci_dev)
{
//驱动信息

	printk("-----------------device info----------------\n");
	printk("vendor:%0x\n", pci_dev->vendor);
    printk("device:%0x\n", pci_dev->device);
    printk("subvendor:%0x\n", pci_dev->subsystem_vendor);
    printk("subdevice:%0x\n", pci_dev->subsystem_device);
	printk("class:%0x\n",pci_dev->class);
	printk("rom_base_reg:%0x\n", pci_dev->rom_base_reg);
    printk("pin:%0x\n", pci_dev->pin);
    printk("irq:%0x\n", pci_dev->irq);
	printk("-----------------device info----------------\n");
	return 0;
}

static int pci_request_addr(struct pci_dev *pci_dev)
//设置io
{
	bar0_io_base = pci_resource_start(pci_dev,0);
	bar0_io_end = pci_resource_end(pci_dev,0);
	bar0_io_length = pci_resource_len(pci_dev,0);
	bar1_io_base = pci_resource_start(pci_dev,1);
	bar1_io_end = pci_resource_end(pci_dev,1);
	bar1_io_length = pci_resource_len(pci_dev,1);
	return 0;

}

static void pci_io_info_print(struct pci_dev *pci_dev)
{

	printk("-------------------io info------------------\n");
	printk("bar%d_io_base:0x%0lx\n", 0, bar0_io_base);
	printk("bar%d_io_end:0x%0lx\n", 0, bar0_io_end);
	printk("bar%d_io_length:0x%0lx\n", 0, bar0_io_length);


	printk("bar%d_io_base:0x%0lx\n", 1, bar1_io_base);
	printk("bar%d_io_end:0x%0lx\n", 1, bar1_io_end);
	printk("bar%d_io_length:0x%0lx\n", 1, bar1_io_length);

	printk("-------------------io info------------------\n");
	
}


static int pci_init_mem(struct pci_dev *pci_dev)
{
	int ret;
	ret = pci_request_region(pci_dev,0,"pci_driver_hessen");
	if(ret != 0 )
	{
		printk("failed pci_request_region\n");
		return 1;
	}else{
		printk("success pci_request_region\n");
		return 0;
	}
	


}

static int pci_exit_mem(struct pci_dev *pci_dev)
{
	pci_release_region(pci_dev,0);
	printk("pci_release_region exectued\n");
	return 0;
}



static int pci_set_map(struct pci_dev *pci_dev)
{
	vir_addr = ioremap(bar0_io_base,bar0_io_length);
	if(vir_addr == 0)
	{
		printk("ioremap failed\n"); 
		return 0;
	}else
	{
		printk("ioremap success\n");
		return 1;
	}
}


static int pci_clean_map(struct pci_dev *pci_dev)
{
	iounmap(vir_addr);
	printk("iounmap executed\n");
	return 0;
}







//读写函数
static ssize_t demo_read(struct file *filp, char __user *buf,size_t size, loff_t *ppos)
{

	return 0;
}

static ssize_t demo_write(struct file *filp, const char __user *buf,size_t size, loff_t *ppos)
{
	return 0;
}
//设备控制函数
static int demo_open(struct inode * nodep, struct file *filp)
{
	printk("demo_io_open success\n");
	return 0;
}





















/*--------------------------------------------------------------*/




static int __init demo_probe(struct pci_dev *pci_dev,const struct pci_device_id *pci_id)
{
/* 驱动发现 */
	printk(KERN_WARNING"THIS IS THE PROBE\n");
	pci_info_print(pci_dev);//传入指针	
	pci_request_addr(pci_dev);
	pci_io_info_print(pci_dev);

	pci_init_mem(pci_dev);
	pci_set_map(pci_dev);

	printk(KERN_WARNING"THIS IS THE PROBE OVER\n");
	return 0;
};

static void __exit demo_remove(struct pci_dev *pci_dev)
{
/* 驱动卸载 */
	printk(KERN_WARNING"THIS IS THE REMOVE\n");
	pci_exit_mem(pci_dev);
	pci_clean_map(pci_dev);


	printk(KERN_WARNING"THIS IS THE REMOVE OVER\n");
};







/* 设备模块驱动 */
static struct pci_driver demo_pci_driver = {
    .name= DEMO_MODULE_NAME,  /* 设备模块名称 */
    .id_table = demo_pci_tbl, /* 能够驱动的设备列表 */
    .probe = demo_probe,      /* 查找并初始化设备 */
    .remove =  demo_remove,   /* 卸载设备模块 */
/* ... */};


/*读写模块 */
static const struct file_operations demo_fops = {
	.owner = THIS_MODULE,
	.read = demo_read,
	.write = demo_write,
	.open = demo_open,
};








static int __init demo_init_module (void)
{
	printk(KERN_WARNING"This is the init module\n");
    return pci_register_driver(&demo_pci_driver);
}

static void __exit demo_cleanup_module (void)
{
	printk(KERN_WARNING"This is the exit module\n");
    pci_unregister_driver(&demo_pci_driver);
}






/* 加载驱动程序模块入口 */
module_init(demo_init_module);

/* 卸载驱动程序模块入口*/
module_exit(demo_cleanup_module);


