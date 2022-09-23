#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/spinlock.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/sched.h>
#include <linux/kthread.h>

MODULE_LICENSE("Dual BSD/GPL");

//static char   message[256] = {0};

#define _GET_DATA   _IOR('M', 1, unsigned long * )
#define _PUT_DATA    _IOR('M', 2, unsigned long * )

#define End_Spawap32(num)        ( ( (num >> 24)&0xff) |   (( (num >> 16)&0xff) << 8) |   (( ( num >> 8)&0xff) << 16) |  (( ( num)&0xff) << 24) )

#define NosEnd_Spawap32(num)        (  (num >> 24) |    (num >> 16) |   ( num >> 8) |  ( num && 0xff000000))

      
void ioremap_gpio(unsigned int *);

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
ssize_t  My_write(struct file *filep,const char __user *buf,size_t count,loff_t *ppos);
int  My_open(struct inode *, struct file *);
static long My_ioctl(struct file *, unsigned int , unsigned long);

static DECLARE_WAIT_QUEUE_HEAD(wq);
int flag = 0;
volatile int  random_delay_1(void);
 
int gCount = 0;
struct list_head myList = LIST_HEAD_INIT(myList);

 //{&myList,&myList};
struct MyData{
   int data;
   int size;
};

struct MyTempDev {

unsigned long size;
struct semaphore sem;
struct cdev cdev;
};

int devnum;
struct file_operations MyTempDevfops = {
  .owner = THIS_MODULE,
  .read = My_read,
  .write = My_write,
  .open = My_open,
  .unlocked_ioctl = My_ioctl
  
};

  dev_t dev;
  int gQDepth = 125;

void printList(struct list_head  *tList);

static int hello_init(void)
{
    int major,minor=0, err = -1;
   // struct MyData sd;
   struct cdev *MyCDev;
   
    alloc_chrdev_region(&dev,minor,1,"MyTempDev");

    MyCDev  = cdev_alloc();
    MyCDev->ops = &MyTempDevfops;
     
    major = MAJOR(dev);
    minor = MINOR(dev);

    printk(KERN_ALERT "Hello Welcome to MyTempDrv  MajNum: %d  ,  MinNum%d \n ",major,minor);
       
    devnum = MKDEV(major,minor);
    err = cdev_add(MyCDev,devnum,1);

     // printk(KERN_ALERT  "Exclusive OR Check %x \n ", 0x1101 ^ 0x0010);

    int i;
    

    printk( KERN_ALERT  "No Cdev Add Error ...Sucess !!!!! \n");
    
    return 0;
}

static void hello_exit(void)
{
    unregister_chrdev_region(dev, 1);
        
    printk(KERN_ALERT "Hello Goodbye, Driver world \n");
}

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     //long int i=0;
     int copied;
     //char addrstr[2560]
     char addrstr[1024] = "Hello Welcome to the device driver program and i am sending to you --- user space ";
     printk(KERN_ALERT "Hello I am at Read Function going to wait \n ");
     
     wait_event_interruptible(wq,flag!=0);
    
     printk(KERN_ALERT "Hello I have woken up from sleep \n ");
     
     copied = copy_to_user(buf ,addrstr, sizeof(addrstr ));
     if(copied > 0)
        printk(KERN_ALERT "Err in copy_to_user  Still to be  Copied  %d Bytes \n ", copied);
   
     return 0;
}


ssize_t My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos)
{
     //int size_of_message;
     printk(KERN_ALERT "Hello I am at Write Function \n");
     
     flag =1;
     wake_up_interruptible(&wq);
     
     return count;
}


static long My_ioctl(struct file *filp, unsigned int  Cmd,  unsigned long  DrvArg){

  volatile unsigned int  nUser;
  unsigned long tUser;

  printk(KERN_ALERT "Hello I am at IOCTL Function  \n");

 switch (Cmd)
    {
        /* Get   request packets */
        case _GET_DATA:
            
                if (copy_to_user(( unsigned long * )DrvArg, &gQDepth, 1));
                {
                    return -EACCES;
                }
               
           break;
        case _PUT_DATA:
            
            copy_from_user( &tUser ,( unsigned long *) DrvArg,4);
            nUser = tUser;
            printk(KERN_ALERT "Recieved address %x   \n",nUser);
            ioremap_gpio(&nUser);
            break;
     }

      //printk(KERN_ALERT "Recieved on IOCTL Bytes  %x \n",nUser);
      
      return 0;

}

int  My_open(struct inode *inode, struct file *filep)
{
      printk(KERN_ALERT "Hello Openning Device .........\n ");
      return 0;
}
 



void ioremap_gpio(unsigned int *nUser){
 int var = 0xdeadbeef;
 unsigned int *gpio1DataAdr;// = 0x0209c000  0x020a0000       0x020a4000     0x020a8000    
 unsigned int *gpio1DirAdr ;  //= 0x0209c004   0x020a0004       0x020a4004     0x020a8004
  
 unsigned int readVal;
 
  printk(KERN_ALERT "Remapping %x \n",*nUser);
  gpio1DataAdr = ioremap(0x7e200000,4);
    
  
   readVal = ioread32(gpio1DataAdr);
   //random_delay_1();
 
   
   /* To stop the beep sound */
   // iowrite32((0xfffffdff & *gpio1DataAdr),gpio1DataAdr);//resetSet  port8  (reset 10th bit of DATA Dword)



    printk(KERN_INFO " The IO read Data reg ADDR %x is %x \n",gpio1DataAdr,*gpio1DataAdr);  
    iounmap( (volatile unsigned int *) 0x7e200000);
    //printk(KERN_INFO " BEEP STOPED IO read Data reg is %x and Io read Dir Reg is %x \n",*gpio1DataAdr,*gpio1DirAdr);
}

volatile  int random_delay_1(void)
{
    int i=0, j=0,k=0;
    
   for (i=0;i<6553;i++)
    {
        for (j=0;j<65536;j++)
          {
               k++;
              }
     }
  printk(KERN_INFO "In delay loop........ ");   

 return 0;
}
module_init(hello_init);
module_exit(hello_exit);

/* 
3033_02D0 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_MDC) 32 R/W 0000_0116h 8.2.5.176/ 1513

3033_02D4 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_MDIO) 32 R/W 0000_0116h 8.2.5.177/ 1515

3033_02D8 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_TD3) 32 R/W 0000_0116h 8.2.5.178/ 1516

3033_02DC Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_TD2) 32 R/W 0000_0116h 8.2.5.179/ 1517

3033_02E0 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_TD1) 32 R/W 0000_0116h 8.2.5.180/ 1518

3033_02E4 Pad Control Register  (IOMUXC_SW_PAD_CTL_PAD_ENET_TD0) 32 R/W 0000_0116h 8.2.5.181/ 1519

3033_02E8 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_TX_CTL) 32 R/W 0000_1916h 8.2.5.182/ 1520

3033_02EC Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_TXC) 32 R/W 0000_0116h 8.2.5.183/1522

3033_02F0 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RX_CTL) 32 R/W 0000_0116h 8.2.5.184/1523

3033_02F4 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RXC) 32 R/W 0000_0116h 8.2.5.185/1524

3033_02F8 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RD0) 32 R/W 0000_0116h 8.2.5.186/1525

3033_02FC Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RD1) 32 R/W 0000_0116h 8.2.5.187/1526

3033_0300 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RD2) 32 R/W 0000_0116h 8.2.5.188/1527

3033_0304 Pad Control Register (IOMUXC_SW_PAD_CTL_PAD_ENET_RD3) 32 R/W 0000_0116h 8.2.5.189/1529
* 
* */
