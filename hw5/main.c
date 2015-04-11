#include <asm/uaccess.h>
#include <linux/workqueue.h>
#include <linux/sched.h>
#include <asm/io.h>
#include <linux/ioport.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/slab.h> 
#include "ioc_hw5.h"
#define DEV_NAME "mydev"
#define DMA_BUFSIZE 64

#define DMASTUIDADDR 0x0                  // ioctl : set and printk value
#define DMARWOKADDR 0x4                 // ioctl : set and printk value 
#define DMAIOCOKADDR 0x8                 // ioctl : set and printk value 
#define DMAIRQOKADDR 0xc                 // ioctl : set and printk value 
#define DMACOUNTADDR 0x10              // ISR : set value, exit_module : printk value
#define DMAANSADDR 0x14                    // work routine : set value, read: printk value 
#define DMAREADABLEADDR 0x18       // ioctl : check value, write : check value
#define DMABLOCKADDR 0x1c               // ioctl: set and printk value, write: check value 
#define DMAOPCODEADDR 0x20           // write: set value, work routine: get value
#define DMAOPERANDBADDR 0x21      // write: set value, work routine: get value
#define DMAOPERANDCADDR 0x25      // write: set value, work routine: get value

#define DEVNUM_COUNT 1 
#define DEVNUM_START 0
MODULE_LICENSE("Dual BSD/GPL");

void *dma_buf;
static int dev_major;
static int dev_minor;
static struct cdev *dev_cdevp;

static int drv_read(struct file *filp, char __user *buffer, size_t, loff_t*);
static int drv_open(struct inode*, struct file*);
static int drv_write(struct file *filp, const char __user *buffer, size_t, loff_t*);
static int drv_release(struct inode*, struct file*);
static int drv_ioctl(struct file *, unsigned int , unsigned long );
unsigned char myinb(unsigned short int port){
	return *(volatile unsigned char*)(dma_buf+port);        
}
unsigned short myinw(unsigned short int port){
	return *(volatile unsigned short*)(dma_buf+port);
}
unsigned int myinl(unsigned short int port){
        return *(volatile unsigned int*)(dma_buf+port);
}
void myoutb(unsigned char data,unsigned short int port){
	*(unsigned char *)(dma_buf+port) = data;
}
void myoutw(unsigned short data,unsigned short int port){
	*(unsigned short *)(dma_buf+port) = data;
}
void myoutl(unsigned int data,unsigned short int port){
   *(unsigned int *)(dma_buf+port) = data;	
  
}
static struct work_struct *work_routine;
struct w_data *wdata;
static struct file_operations fops = {
    open:  drv_open,
    release: drv_release,
    read:  drv_read,
    write: drv_write,
    unlocked_ioctl: drv_ioctl,
    owner:  THIS_MODULE,
};
struct w_data {
   unsigned char a;
   unsigned int b;
   unsigned short c;
};
int prime(int base, short nth)
{
    int fnd=0;
    int i, num, isPrime;

    num = base;
    while(fnd != nth) {
        isPrime=1;
        num++;
        for(i=2;i<=num/2;i++) {
            if(num%i == 0) {
                isPrime=0;
                break;
            }
        }

        if(isPrime) {
            fnd++;
        }
    }
    return num;
}
static void arithmetic_routine(struct work_struct *ws)
{
	unsigned char opa = myinb(DMAOPCODEADDR);
	unsigned int opb = myinl(DMAOPERANDBADDR);
	unsigned short opc = myinw(DMAOPERANDCADDR);
	int ans = 0;
	
	switch(opa) {
        case '+':
            ans=opb+opc;
            break;
        case '-':
            ans=opb-opc;
            break;
        case '*':
            ans=opb*opc;
            break;
        case '/':
            ans=opb/opc;
            break;
        case 'p':
            ans = prime(opb, opc);
            break;
        default:
            ans=0;
	    break;
	
	}
	printk("OS_HW5:drv_arithmetic_routine():%d %c %d = %d\n",opb,opa,opc,ans);
	myoutl(ans,DMAANSADDR);
	if(myinl(DMABLOCKADDR==0))
		myoutl(1,DMAREADABLEADDR);
}
static int drv_open(struct inode *inode, struct file *filp)
{
	try_module_get(THIS_MODULE);
	printk("OS_HW5:drv_open(): device open\n");
	return 0;
}

static int drv_release(struct inode *inode,struct file *filp)
{
	module_put(THIS_MODULE);
	printk("OS_HW5:drv_release(): device close\n");
	return 0;
}
static int drv_read(struct file *filp,char __user *buffer,size_t length,loff_t *offset)
{
   	printk("OS_HW5:drv_read(): ans = %d\n",myinl(DMAANSADDR));
    	myoutl(0,DMAANSADDR);
	myoutl(0,DMAREADABLEADDR);
	return 0;
}
static int drv_write(struct file *filp, const char __user *buffer, size_t length, loff_t *offset)
{
	int mode = myinl(DMABLOCKADDR);
	printk("OS_HW5:drv_write(): queue work\n");	
	INIT_WORK(work_routine,arithmetic_routine);
	wdata = (struct w_data*)buffer;
        myoutb(wdata->a,DMAOPCODEADDR);
        myoutw(wdata->c,DMAOPERANDCADDR);
        myoutl(wdata->b,DMAOPERANDBADDR);
	if(mode){      	//blocking IO
		printk("OS_HW5:drv_write(): block\n");		
		schedule_work(work_routine);
		flush_scheduled_work();
	}
	else{             //Nonblocking IO
                schedule_work(work_routine);
	}
	return 0;
}
static int drv_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int val = 1;
	unsigned int x_stuid = 101062206;
	unsigned int o_stuid = 0;
	switch(cmd){
		case HW5_IOCSETSTUID:
			get_user(val,(int *)arg);
			myoutl(x_stuid,DMASTUIDADDR);
			o_stuid= myinl(DMASTUIDADDR);
			printk("OS_HW5:drv_ioctl(): My STUID is = %d\n",o_stuid);
			break;
		case HW5_IOCSETRWOK:
			get_user(val,(int *)arg);
			myoutl(val,DMARWOKADDR);
			printk("OS_HW5:drv_ioctl(): RW OK\n");	
			break;
		case HW5_IOCSETIOCOK:
			get_user(val,(int *)arg);
			myoutl(val,DMAIOCOKADDR);
			printk("OS_HW5:drv_ioctl(): IOC OK\n");
			break;
		case HW5_IOCSETIRQOK:
			get_user(val,(int *)arg);
			myoutl(val,DMAIRQOKADDR);
			printk("OS_HW5:drv_ioctl(): IRC OK\n");
			break;
		case HW5_IOCSETBLOCK:
			get_user(val,(int *)arg);
			myoutl(val,DMABLOCKADDR);
			if(val)	
				printk("OS_HW5:drv_ioctl(): Blocking IO\n");
			else 
				printk("OS_HW5:drv_ioctl(): Non-Blocking IO\n");
			break;
		case HW5_IOCWAITREADABLE:
			
			while(myinl(DMAREADABLEADDR)!=1)
				msleep(1000);				
			put_user(1,(int *)arg);
			printk("OS_HW5:drv_ioctl(): wait readable 1\n");	
			break;
		default:
			return -1;


	}
	
	return 0;
}
static int init_dev(void) {
	dev_t dev;
	printk("OS_HW5:init_modules():...............Start...............\n");	
	dev_cdevp = cdev_alloc();
	

	if(alloc_chrdev_region(&dev,DEVNUM_START,DEVNUM_COUNT,DEV_NAME)){
    		printk("Register char dev error\n");
    		return -1;
   	}
	else{
		printk("OS_HW5:init_modules(): register chrdev(%i,%i)\n",MAJOR(dev),MINOR(dev));

	}
	dev_major = MAJOR(dev);
	dev_minor = MINOR(dev);
	
	cdev_init(dev_cdevp,&fops);
	if(cdev_add(dev_cdevp, MKDEV(dev_major,dev_minor), 1)){
        	printk(KERN_ALERT"Add char dev error!\n");
    	}	
	
	dma_buf = kzalloc(DMA_BUFSIZE,GFP_KERNEL);
	if(dma_buf){
		printk("OS_HW5:init_modules(): allocate dma buffer\n");
	}
	work_routine = kmalloc(sizeof(typeof(*work_routine)), GFP_KERNEL);
	return 0;

}
static void exit_dev(void) {
	unregister_chrdev_region(MKDEV(dev_major,dev_minor),DEVNUM_COUNT);
        cdev_del(dev_cdevp);
	kfree(dma_buf);
	printk("OS_HW5:exit_modules(): free dma buffer\n");
	kfree(work_routine);
	printk("OS_HW5:exit_modules(): unregister chrdev\n");
	printk("OS_HW5:exit_modules():..............End..............\n");
}

module_init(init_dev);
module_exit(exit_dev);
