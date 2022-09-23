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

#define _GET_DATA   _IOW('M', 1, unsigned long  )
#define _PUT_DATA    _IOR('M', 2, unsigned long  )

#define End_Spawap32(num)        ( ( (num >> 24)&0xff) |   (( (num >> 16)&0xff) << 8) |   (( ( num >> 8)&0xff) << 16) |  (( ( num)&0xff) << 24) )

#define NosEnd_Spawap32(num)        (  (num >> 24) |    (num >> 16) |   ( num >> 8) |  ( num && 0xff000000))

struct list_head queue_list_pack;
LIST_HEAD(queue_list_pack);
      
void ioremap_gpio(vod);
int extarct_packet(void);
ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
ssize_t  My_write(struct file *filep,const char __user *buf,size_t count,loff_t *ppos);
int  My_open(struct inode *, struct file *);
static long My_ioctl(struct file *, unsigned int , unsigned long);
int My_fasync(int, struct file *,int);
void ListHeadEntryDemo(unsigned int *dataPacket);
void printListEntry(struct list_head  *tList);

void ListHeadDemo(unsigned int *);
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
  .unlocked_ioctl =  My_ioctl,
  .fasync = My_fasync
};

  dev_t dev;
  int gQDepth = 125;
   struct fasync_struct  *async_q[10];
int thread_init(char *);

 
struct  my_mctp_struct {
    unsigned int *mctp_packet;
    unsigned char mctp_pkt_len;
    struct list_head queue;
}  ;

struct list_head todo_list;
LIST_HEAD(todo_list);

struct list_head gPacket ;
LIST_HEAD(gPacket);

void printList(void);


static int hello_init(void)
{
   int major,minor=0, err = -1;

   struct cdev *MyCDev;
   struct device_node *np;
  
    alloc_chrdev_region(&dev,minor,1,"MyTempDev");

    MyCDev  = cdev_alloc();
    MyCDev->ops = &MyTempDevfops;
    MyCDev->owner = THIS_MODULE;
 
    major = MAJOR(dev);
    minor = MINOR(dev);

    printk(KERN_ALERT "Hello Welcome to MyTempDrv  MajNum: %d  ,  MinNum%d \n ",major,minor);
       
    devnum = MKDEV(major,minor);
    err = cdev_add(MyCDev,devnum,1);

     // printk(KERN_ALERT  "Exclusive OR Check %x \n ", 0x1101 ^ 0x0010);
  int i;
 
  for (i= 0 ;i<5;i++)
   { //ListHeadEntryDemo(&i);     
     ListHeadDemo(&i);
   }
   printList(); 
  //for (i= 0 ;i<10;i++)
  // printListEntry(&todo_list);

   if (err)
     printk(KERN_ALERT " Errrrror ");    

   printk( KERN_ALERT  "No Cdev Add Error ...Sucess !!!!! \n");
    
   return 0;
}


void ListHeadDemo(unsigned int *dataPacket){
  struct my_mctp_struct  *ptrPacket;
   //INIT_LIST_HEAD(&gPacket);

   ptrPacket = kmalloc (sizeof(struct my_mctp_struct), GFP_KERNEL);

    if(ptrPacket == NULL)
    {
        printk("ERROR: PAcket driver Out of memory \n");
        return;
    }
   
   INIT_LIST_HEAD(&ptrPacket->queue);
   //printk(KERN_INFO "List left is 0x %x and List right is %x and main list is %x \n",ptrPacket->queue.prev,ptrPacket->queue.next,ptrPacket->queue);
 
    ptrPacket->mctp_packet =  kmalloc (sizeof(unsigned int ), GFP_KERNEL);

    if(ptrPacket->mctp_packet == NULL)
    {
        printk("ERROR: mctp_packet Out of memory \n");
        return;
    }
   

   ptrPacket->mctp_packet = dataPacket;

   printk(KERN_INFO "Data packet added is %d \n" , *(ptrPacket->mctp_packet));
   
   list_add_tail(&ptrPacket->queue, &gPacket);

  }
  
  
  void printList(void){
  int i;
   struct my_mctp_struct  *tDataPacket;
   struct my_mctp_struct *tTempPacket;
   printk(KERN_INFO "In Print List Functon to run list_for_each_entry_safe \n"); 

     list_for_each_entry_safe(tDataPacket, tTempPacket,&gPacket, queue)
    {
        printk(KERN_INFO "Packet Data is %d and packet addtess is 0x%x \n",*(tDataPacket->mctp_packet),tDataPacket); 
        
        list_del(&tDataPacket->queue);
       // kfree(
     }
     
     
 #if 0
    list_for_each(Packet, &tPacket->queue) {
     tPacket = list_entry(Packet, struct my_mctp_struct,queue);
     //printk(KERN_INFO "Packet Data is %d \n",*(tPacket->mctp_packet)); 
    }
 #endif
}
 
static long My_ioctl(struct file *filp, unsigned int  Cmd,  unsigned long DrvArg){

  unsigned long  nUser = 0;

  printk(KERN_ALERT "Hello I am at IOCTL Function %x  \n",DrvArg);

 switch (Cmd)
    {
        /* Get   request packets */
        case _GET_DATA:
            printk(KERN_ALERT "Hello I am at IOCTL case  \n");
            
           //kill_fasync(&async_q,SIGIO,POLL_IN);

              if (copy_from_user( &nUser , &DrvArg,sizeof(unsigned long) ))
            {
                printk(KERN_ALERT "Unable to copy from user  \n");
                return -EFAULT;
             }
             
           //kill_fasync(&async_q,SIGIO,POLL_IN);

          /* if (copy_to_user(( unsigned long * )DrvArg, &gQDepth, 4))
                {
                    return -EACCES;
                } */
               
           break;
        case _PUT_DATA:
            if (copy_from_user( &nUser ,( unsigned long *)DrvArg,1 ))
            {
                return -EFAULT;
             }
           
            break;
     }

      printk(KERN_ALERT "Recieved on IOCTL Bytes  %x \n",nUser);

 return 0;

}

int  My_open(struct inode *inode, struct file *filep)
{
     struct cdev *mycdev;
     int major,minor;
     dev_t tdev;
     
     mycdev = inode->i_cdev;
     tdev  = mycdev->dev;

     major = MAJOR(tdev);
     minor = MINOR(tdev);
     
     filep->private_data = mycdev;

      printk(KERN_ALERT "Hello Openning Device with Major  %d and minor %d .........\n ",major,minor);
      return 0;
}
 
int My_fasync(int fd, struct file *filep, int mode) {

     struct cdev *mycdev;
     int major,minor;
     dev_t tdev;
        
     mycdev = ( struct cdev *)  filep->private_data;
     tdev  = mycdev->dev;

     major = MAJOR(tdev);
     minor = MINOR(tdev);
  printk(KERN_ALERT "In fasync to signal the Device with Major  %d and minor %d .........\n ",major,minor);
 //return  fasync_helper(fd,filep,mode,&async_q);
   return 0;
}

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     //long int i=0;
     int copied;
     //char addrstr[2560]
     char addrstr[1024] = "Hello Welcome to the device driver program and i am sending to you --- user space ";
     printk(KERN_ALERT "Hello I am at Read Function \n ");
    
    //spin_lock(&tLkp);
     copied = copy_to_user(buf ,addrstr, 1 );
     if(copied > 0)
        printk(KERN_ALERT "Err in copy_to_user  Still to be  Copied  %d Bytes \n ", copied);
   //spin_unlock(&tLkp);
     return 0;
}


ssize_t My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos)
{
     //int size_of_message;
     printk(KERN_ALERT "Hello I am at Write Function \n");
     
     return count;
}


void my_fsaync_handler(int fd, struct file *filp, int mode) {



}

void My_List_Replace( struct list_head  *old,struct list_head  *new) {

     new->next =  old->next;
     new->prev =  old->prev;
     //old->next->prev = new;
     new->next->prev = new;
    //old->next->prev = new;
     new->prev->next = new;
 
     INIT_LIST_HEAD(old);

}

void My_List_Del( struct list_head  *entry)  {


}

static void hello_exit(void)
{
    
    unregister_chrdev_region(dev, 1);
     printk(KERN_ALERT "Hello Goodbye, Driver world \n");
}


module_init(hello_init);
module_exit(hello_exit);
