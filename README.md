# OperationSys_repo
FPGA acceleration demo repo,including PCI drivers, IO controler, user_base programs




PCI驱动框架文档
V1.0



Author：杜海鑫

Date：2018.9.10







概述









Linux 下驱动作为系统的模块，在内核的支持下可以灵活的组织，总线驱动为pci子系统提供了支持，在pci驱动编写时会更便捷和安全。

Linux底层代码发展变化很快，此文档作为pci驱动框架，是一个摸索过程的文档，对编写驱动提供一个相对健全的方式，欢迎补充和提出问题


内核版本4.15






Github 仓库地址：https://github.com/dhxsy1994/OperationSys_rep

要进行Linux驱动开发我们首先要有Linux内核的源码树，在不同版本的Linux内核所对应的系统接口可能不同。
PCI子系统作为计算机总线的一部分，有独立的寻址功能，能够去帮我们做很多工作，但PCI子系统的设备仍然需要我们的驱动去启动，PCI子系统用来管理PCI插槽上的设备，向内核请求管理的同时要注册pci和pci设备（字符设备，块设备，网络设备）



驱动基于内核

一、内核模块

驱动作为Linux模块可以灵活的安装和卸载，如有需要可以对内核配置其跟随系统启动，或者手动安装和卸载模块，下面是内核模块的代码实现

···
#include <linux/kernel.h> /*Needed by all modules*/
#include <linux/module.h> /*Needed for KERN_* */
#include <linux/init.h> /* Needed for the macros */

MODULE_LICENSE("GPL");

static int year=2014;

static int hello_init(void)//
{
  printk(KERN_WARNING "Hello kernel, it's %d!\n",year);
  return 0;
}


static void hello_exit(void)
{
  printk("Bye, kernel!\n");
}

/* main module function*/进入退出函数，关键
module_init(hello_init);
module_exit(hello_exit);
···c




二、Makefile 编译

···c
obj-m :=MOUDLE.o                           #产生模块的目标文件
                                                                                                                                                                all:
make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) modules    #编译模块
clean:
make -C /lib/modules/$(shell uname -r)/build SUBDIRS=$(PWD) clean      #清理





三、管理内核模块


	$ lsmod 
查看已安装的内核模块

	$ rmmod
卸载模块

	$ insmod
安装模块



PCI信息查看

一、PCI 设备查看

	$ lspci
查看由系统提供的发现程序所列出的PCI设备

回显内容为所有的pci设备，pci桥设备ISA设备等等






	$ lspci -vmm
显示设备详细厂商信息

回显如下
	
Slot:	04:00.0
Class:	Ethernet controller
Vendor:	Realtek Semiconductor Co., Ltd.
Device:	RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller
SVendor:	Realtek Semiconductor Co., Ltd.
SDevice:	RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller
Rev:	06


	$ lspci -vnn
显示设备正在使用的驱动模块，以及vendor id ，subdevice id，驱动编写会用到


二、PCI 设备驱动卸载

如果pci设备正在被系统所提供的驱动运行，需要手动卸载模块再安装我们所编译好的驱动模块才能使用。

下面是使用pci_hessen的屏显


04:00.0 Ethernet controller [0200]: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:8168] (rev 06)
	Subsystem: Realtek Semiconductor Co., Ltd. RTL8111/8168/8411 PCI Express Gigabit Ethernet Controller [10ec:0123]
	Flags: bus master, fast devsel, latency 0, IRQ 18
	I/O ports at e000 [size=256]
	Memory at f0004000 (64-bit, prefetchable) [size=4K]
	Memory at f0000000 (64-bit, prefetchable) [size=16K]
	Capabilities: <access denied>
	Kernel driver in use: pci_hessen
	Kernel modules: r8169






PCI驱动框架
一、基础PCI系统结构体

/* 设备模块信息 */
static struct pci_driver demo_pci_driver = {
    .name= DEMO_MODULE_NAME,  /* 设备模块名称 */
    .id_table = demo_pci_tbl, /* 能够驱动的设备列表 */
    .probe = demo_probe,      /* 查找并初始化设备 */
    .remove =  demo_remove,   /* 卸载设备模块 */
/* ... */};

PCI模块最为重要的一个部分
驱动入口和出口函数，以及能驱动的设备列表



二、PCI设备注册框架代码


该空驱动程序可以注册硬件到PCI子系统

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




MODULE_LICENSE("GPL");
#define DEMO_MODULE_NAME "pci_hessen"

#define PCI_VENDOR_ID_DEMO 0x10ec
#define PCI_DEVICE_ID_DEMO 0x8168


/* 指明该驱动程序适用于哪一些PCI设备 */
static struct pci_device_id demo_pci_tbl[] ={
    {PCI_DEVICE(PCI_VENDOR_ID_DEMO,PCI_DEVICE_ID_DEMO),},
    {0,},
};



static int __init demo_probe(struct pci_dev *pci_dev,const struct pci_device_id *pci_id)
{
/*发现操作*/
	printk(KERN_WARNING"THIS IS THE PROBE\n");

	return 0;
};

static void __exit demo_remove(struct pci_dev *pci_dev)
{
/* 卸载操作 */
	printk(KERN_WARNING"THIS IS THE REMOVE\n");


};


/* 设备模块信息 */
static struct pci_driver demo_pci_driver = {
    .name= DEMO_MODULE_NAME,  /* 设备模块名称 */
    .id_table = demo_pci_tbl, /* 能够驱动的设备列表 */
    .probe = demo_probe,      /* 查找并初始化设备 */
    .remove =  demo_remove,   /* 卸载设备模块 */
/* ... */};



static int __init demo_init_module (void)
{
/* allocate (several) major number */
    //ret = alloc_chrdev_region(&devno, 0, MAX_DEVICE, "buffer"); 
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

注：PCI_VENDOR_ID_DEMO， PCI_DEVICE_ID_DEMO 的值可以通过设备资料和lspci命令获得16进制的值

三、Demo_Probe 操作

static int __init demo_probe(struct pci_dev *pci_dev,const struct pci_device_id *pci_id)                                                                                                                                                 
{
/* 驱动发现 */
    printk(KERN_WARNING"THIS IS THE PROBE\n");
    if (pci_enable_device(pci_dev))//初始化设备
    {
        printk("fail to enable_device\n");
         return -EIO;
    }
    
    pci_info_print(pci_dev);//传入指针  
    pci_request_addr(pci_dev);//
    pci_io_info_print(pci_dev);

    pci_init_mem(pci_dev);//
    pci_set_map(pci_dev);//
    set_dma(pci_dev);//
    
    chardev_init();
    printk(KERN_WARNING"THIS IS THE PROBE OVER\n");
     return 0;
 };

四、pci_dev结构体

该结构体由内核提供支持，无需显式定义

struct pci_dev {
    struct list_head bus_list;    /* node in per-bus list对应总线下所有的设备链表 */
    struct pci_bus    *bus;        /* bus this device is on   设备所在的bus*/
    struct pci_bus    *subordinate;    /* bus this device bridges to 当作为PCI桥设备时，指向下级bus */

    void        *sysdata;    /* hook for sys-specific extension */
    struct proc_dir_entry *procent;    /* device entry in /proc/bus/pci */
    struct pci_slot    *slot;        /* Physical slot this device is in */

    unsigned int    devfn;        /* encoded device & function index 设备号和功能号*/
    unsigned short    vendor;
    unsigned short    device;
    unsigned short    subsystem_vendor;
    unsigned short    subsystem_device;
    unsigned int    class;        /* 3 bytes: (base,sub,prog-if) */
    u8        revision;    /* PCI revision, low byte of class word */
    u8        hdr_type;    /* PCI header type (`multi' flag masked out) */
    u8        pcie_cap;    /* PCI-E capability offset */
    u8        msi_cap;    /* MSI capability offset */
    u8        msix_cap;    /* MSI-X capability offset */
    u8        pcie_mpss:3;    /* PCI-E Max Payload Size Supported */
    u8        rom_base_reg;    /* which config register controls the ROM */
    u8        pin;          /* which interrupt pin this device uses */
    u16        pcie_flags_reg;    /* cached PCI-E Capabilities Register */

    struct pci_driver *driver;    /* which driver has allocated this device */
    u64        dma_mask;    /* Mask of the bits of bus address this
                       device implements.  Normally this is
                       0xffffffff.  You only need to change
                       this if your device has broken DMA
                       or supports 64-bit transfers.  */

    struct device_dma_parameters dma_parms;

    pci_power_t     current_state;  /* Current operating state. In ACPI-speak,
                       this is D0-D3, D0 being fully functional,
                       and D3 being off. */
    u8        pm_cap;        /* PM capability offset */
    unsigned int    pme_support:5;    /* Bitmask of states from which PME#
                       can be generated */
    unsigned int    pme_interrupt:1;
    unsigned int    pme_poll:1;    /* Poll device's PME status bit */
    unsigned int    d1_support:1;    /* Low power state D1 is supported */
    unsigned int    d2_support:1;    /* Low power state D2 is supported */
    unsigned int    no_d1d2:1;    /* D1 and D2 are forbidden */
    unsigned int    no_d3cold:1;    /* D3cold is forbidden */
    unsigned int    d3cold_allowed:1;    /* D3cold is allowed by user */
    unsigned int    mmio_always_on:1;    /* disallow turning off io/mem
                           decoding during bar sizing */
    unsigned int    wakeup_prepared:1;
    unsigned int    runtime_d3cold:1;    /* whether go through runtime
                           D3cold, not set for devices
                           powered on/off by the
                           corresponding bridge */
    unsigned int    d3_delay;    /* D3->D0 transition time in ms */
    unsigned int    d3cold_delay;    /* D3cold->D0 transition time in ms */

#ifdef CONFIG_PCIEASPM
    struct pcie_link_state    *link_state;    /* ASPM link state. */
#endif

    pci_channel_state_t error_state;    /* current connectivity state */
    struct    device    dev;        /* Generic device interface */

    int        cfg_size;    /* Size of configuration space */

    /*
     * Instead of touching interrupt line and base address registers
     * directly, use the values stored here. They might be different!
     */
    unsigned int    irq;
    struct resource resource[DEVICE_COUNT_RESOURCE]; /* I/O and memory regions + expansion ROMs */

    bool match_driver;        /* Skip attaching driver */
    /* These fields are used by common fixups */
    unsigned int    transparent:1;    /* Transparent PCI bridge */
    unsigned int    multifunction:1;/* Part of multi-function device */
    /* keep track of device state */
    unsigned int    is_added:1;
    unsigned int    is_busmaster:1; /* device is busmaster */
    unsigned int    no_msi:1;    /* device may not use msi */
    unsigned int    block_cfg_access:1;    /* config space access is blocked */
    unsigned int    broken_parity_status:1;    /* Device generates false positive parity */
    unsigned int    irq_reroute_variant:2;    /* device needs IRQ rerouting variant */
    unsigned int     msi_enabled:1;
    unsigned int    msix_enabled:1;
    unsigned int    ari_enabled:1;    /* ARI forwarding */
    unsigned int    is_managed:1;
    unsigned int    is_pcie:1;    /* Obsolete. Will be removed.
                             Use pci_is_pcie() instead */
    unsigned int    needs_freset:1; /* Dev requires fundamental reset */
    unsigned int    state_saved:1;
    unsigned int    is_physfn:1;
    unsigned int    is_virtfn:1;
    unsigned int    reset_fn:1;
    unsigned int    is_hotplug_bridge:1;
    unsigned int    __aer_firmware_first_valid:1;
    unsigned int    __aer_firmware_first:1;
    unsigned int    broken_intx_masking:1;
    unsigned int    io_window_1k:1;    /* Intel P2P bridge 1K I/O windows */
    pci_dev_flags_t dev_flags;
    atomic_t    enable_cnt;    /* pci_enable_device has been called */

    u32        saved_config_space[16]; /* config space saved at suspend time */
    struct hlist_head saved_cap_space;
    struct bin_attribute *rom_attr; /* attribute descriptor for sysfs ROM entry */
    int rom_attr_enabled;        /* has display of the rom attribute been enabled? */
    struct bin_attribute *res_attr[DEVICE_COUNT_RESOURCE]; /* sysfs file for resources */
    struct bin_attribute *res_attr_wc[DEVICE_COUNT_RESOURCE]; /* sysfs file for WC mapping of resources */
#ifdef CONFIG_PCI_MSI
    struct list_head msi_list;
    struct kset *msi_kset;
#endif
    struct pci_vpd *vpd;
#ifdef CONFIG_PCI_ATS
    union {
        struct pci_sriov *sriov;    /* SR-IOV capability related */
        struct pci_dev *physfn;    /* the PF this VF is associated with */
    };
    struct pci_ats    *ats;    /* Address Translation Service */
#endif
    phys_addr_t rom; /* Physical address of ROM if it's not from the BAR */
    size_t romlen; /* Length of ROM if it's not from the BAR */
};

五、Demo_Probe 实现

该部分内容包含了字符设备的注册，在字符设备注册完毕时，系统才会在设备文件夹下产生一个字符设备节点

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
 static int pci_init_mem(struct pci_dev *pci_dev)
 {//请求内存
     int ret;
     ret = pci_request_region(pci_dev,0,"pci_driver_hessen");
     if(ret != 0 )
     {
         printk("failed pci_request_region\n");
         request_status = false;
         return 1;
     }else{
         printk("success pci_request_region\n");
         request_status = true;
         return 0;
     }
 }
 static int pci_set_map(struct pci_dev *pci_dev)
 { //io映射
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


static int set_dma(struct pci_dev *pci_dev)
 {
  //DMA设置
     int ret;
     pci_set_master(pci_dev);
     ret = pci_set_dma_mask(pci_dev,0xFFFFFFFFULL);
     if(ret != 0 )
     {
         printk("No useable DMA config\n");
         return 1;
     }
     printk("pci_set_dma success\n");
     return 0;
 }

static int chardev_init(void)
{//字符设备注册       
     mymajor = register_chrdev(0,CHARDEV_NAME,&demo_fops);
//自动分配设备编号，在更早版本的内核中调用方法不同
     if(mymajor < 0)
     {
         printk("register char_dev failed\n");
         return -1;
     }else
     {   
         printk("register char_dev success,the num is %d",mymajor);
         return mymajor;
     }
 
}

字符设备所注册的io读写结构体
 static const struct file_operations demo_fops = {
     .owner = THIS_MODULE,
     .read = demo_read,
     .write = demo_write,
     .open = demo_open,
     .release = demo_release,
 };

六、读写实现


//读写函数
static ssize_t demo_read(struct file *filp, char __user *buf,size_t size, loff_t *ppos)
{
     if(request_status == true)
     {   
         read_buf = ioread32(vir_addr);
         printk("Value at (0x%0x):0x%X\n", vir_addr,read_buf);
         printk("demo_read success\n");
         return 0;
     }else
     {   
         printk("request_status is false\n");
         return 1;
     }
     int ret;
     ret = copy_to_user(buf,&read_buf,sizeof(long));
     if(ret != 0)
     {   
         printk("copy_to_user failed\n");
         return -EFAULT;
     }
     return sizeof(long);
}
 
static ssize_t demo_write(struct file *filp, const char __user *buf,size_t size, loff_t *ppos)
{
     int n;
     int ret;
     ret = copy_from_user(&write_buf,buf,sizeof(long));
     if(ret != 0)
     {   
         printk("copy_from_user failed\n")
         return -EFAULT;
     }
 
}




七、Demo_remove 操作

 static void __exit demo_remove(struct pci_dev *pci_dev)
 {
 /* 驱动卸载 */
     printk(KERN_WARNING"THIS IS THE REMOVE\n");
     pci_exit_mem(pci_dev);
     pci_clean_map(pci_dev);
 
 
     chardev_exit();
     
     
     pci_disable_device(pci_dev);
     
     printk(KERN_WARNING"THIS IS THE REMOVE OVER\n");
 };


八、Demo_remove 实现

static int pci_exit_mem(struct pci_dev *pci_dev)
 {
     pci_release_region(pci_dev,0);
     printk("pci_release_region exectued\n");
     return 0;
 }

static int pci_clean_map(struct pci_dev *pci_dev)
 {
     iounmap(vir_addr);
     printk("iounmap executed\n");
     return 0;
 }
static int chardev_exit()
{
    unregister_chrdev(mymajor,CHARDEV_NAME);
    printk("chardev_exit exectued\n");
    return 0;
}
