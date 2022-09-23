#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/pagemap.h>
#include <linux/string.h>

MODULE_LICENSE("Dual BSD/GPL");

static char   message[256] = {0};

#define _GET_DATA   _IOR('M', 1, unsigned long * )
#define _PUT_DATA   _IOR('M', 2, unsigned long * )

#define End_Spawap32(num)        ( ( (num >> 24)&0xff) |   (( (num >> 16)&0xff) << 8) |   (( ( num >> 8)&0xff) << 16) |  (( ( num)&0xff) << 24) )

#define NosEnd_Spawap32(num)        (  (num >> 24) |    (num >> 16) |   ( num >> 8) |  ( num && 0xff000000))

struct list_head queue_list_pack;
LIST_HEAD(queue_list_pack);
      
int extarct_packet(void);
ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
ssize_t  My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos);
int  My_open(struct inode *, struct file *);
//static int My_ioctl(struct inode *,struct file *, unsigned int , unsigned long);
long My_ioctl (struct file *, unsigned int, unsigned long);
int My_fasync(int, struct file *,int);


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
  .owner = THIS_MODULE,  .read = My_read,
  .write = My_write,
  .open = My_open,
  .unlocked_ioctl = My_ioctl,
  .fasync = My_fasync
};

  dev_t dev;
  int gQDepth = 125;

static int hello_init(void)
{
    int major,minor=0, err;
    struct MyData sd;

    alloc_chrdev_region(&dev,minor,1,"MyTempDev");

    struct cdev *MyCDev = cdev_alloc();
    MyCDev->ops = &MyTempDevfops;
     
    major = MAJOR(dev);
    minor = MINOR(dev);

    printk(KERN_ALERT "Hello Welcome to MyTempDrv  MajNum: %d  ,  MinNum%d \n ",major,minor);
        
    devnum = MKDEV(major,minor);
    err = cdev_add(MyCDev,devnum,1);
   
    if (err)
      printk(KERN_ALERT " Errrrror ");    

    printk( KERN_ALERT  "No Cdev Add Error ...Sucess !!!!! \n");
    
    return 0;
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Hello Goodbye, Driver world \n");
}

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     long int i=0;
     printk(KERN_ALERT "Hello I am at Read Function \n ");
    
     return 0;
}


ssize_t My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos)
{
   int size_of_message,i;
   int res;
   unsigned long uaddr;
   char addrstr[80];
   char kernelData[40960];
   struct page *page;
   char *my_page_address;
   unsigned long copied;
   
    copied = copy_from_user(addrstr, buf, sizeof(addrstr));
    if (copied != 0)
    {
       pr_err("In Kernel to be copied is %d, but only copied %lu\n", count, copied);
    }
    
    uaddr = simple_strtoul(addrstr, NULL, 0);
    down_read(&current->mm->mmap_lock);
    //res = get_user_pages(current, current->mm, uaddr, 10,  1,  1, &page, NULL); applicable for Kernel v3.14
    res = get_user_pages(uaddr, 10,  1, &page, NULL);
    printk(KERN_ALERT "Got the %d Pages from process with id %d \n",res,current->pid);
    
    if (res == 10) 
    {
       //pr_err("Got page\n");
       /* Do something with it */
       my_page_address = kmap(page);
       for(i=0;i<4096;i++)
         kernelData[i] = 0xab;
       for(i=0;i<4096;i++)
         my_page_address[i] = 0xab;
       //strcpy (my_page_address, "Hello,I am sending these data from Kernel spac ethrough page map  \n");
       pr_err("Got address %p and user told me it was %lx\n",my_page_address, uaddr);
       //pr_err("Wrote: %s", my_page_address);        
       kunmap(page);        
       /* Clean up */
      if (!PageReserved(page))
         SetPageDirty(page);
	  //  page_cache_release(page);
    } 
    else {
        printk(KERN_ALERT "Couldn't get page :(\n");
    }
   
    up_read(&current->mm->mmap_lock);
    
    return count;
       
  }

//static int My_ioctl(struct inode *iNode,struct file *filp, unsigned int  Cmd,  unsigned long DrvArg){
long My_ioctl (struct file *filp, unsigned int Cmd, unsigned long DrvArg) {

  unsigned long  nUser;

  printk(KERN_ALERT "Hello I am at IOCTL Function  \n");

 switch (Cmd)
    {
        /* Get   request packets */
        case _GET_DATA:
            
                if (copy_to_user(( unsigned long * )DrvArg, &gQDepth, 4))
                {
                    return -EACCES;
                }
               
           break;
        case _PUT_DATA:
           if (copy_from_user( &nUser ,( unsigned long *) DrvArg,1 ))
            {
                return -EFAULT;
             }
           
            break;
     }

      printk(KERN_ALERT "Recieved on IOCTL Bytes  %d \n",nUser);
 return nUser;
}

int  My_open(struct inode *inode, struct file *filep)
{
      printk(KERN_ALERT "Hello Openning Device .........\n ");
      return 0;
}
 

int My_fasync(int fd, struct file *filep, int mode) {
  return 0;
}

module_init(hello_init);
module_exit(hello_exit);
