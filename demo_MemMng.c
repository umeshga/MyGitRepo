#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/mm.h>

MODULE_LICENSE("Dual BSD/GPL");
//#define PAGE_FAULT_OLD  

#define _GET_DATA   _IOR('M', 1, unsigned long * )
#define _PUT_DATA    _IOW('M', 2, unsigned long * )
#define End_Spawap32(num)        ( ( (num >> 24)&0xff) |   (( (num >> 16)&0xff) << 8) |   (( ( num >> 8)&0xff) << 16) |  (( ( num)&0xff) << 24) )
#define NosEnd_Spawap32(num)        (  (num >> 24) |    (num >> 16) |   ( num >> 8) |  ( num && 0xff000000))

struct list_head queue_list_pack;
int devnum;
dev_t dev;
 int gQDepth = 125;
LIST_HEAD(queue_list_pack);
      
ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
ssize_t  My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos);
//ssize_t  My_write(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
int  My_open(struct inode *, struct file *);
int my_release(struct inode *, struct file *);
//static long My_ioctl(struct file *, unsigned int , unsigned int);
long My_ioctl (struct file *, unsigned int, unsigned long);
int fopsmmap(struct file *filep,struct vm_area_struct *vma) ;
int My_fasync(int, struct file *,int);
void mymap_open(struct vm_area_struct *);
void mymap_close(struct vm_area_struct *);
//int mymap_fault(struct vm_area_struct *vma, struct vm_fault *vmf);
vm_fault_t mymap_fault(struct vm_fault *vmf);
int mymap_nopage(struct vm_area_struct *vma,unsigned long address, int *type);

struct MyData{
    int data;
    int size;
};
struct MyTempDev {
    unsigned long size;
    struct semaphore sem;
    struct cdev cdev;
};

struct file_operations MyTempDevfops = {
  .owner = THIS_MODULE,
  .read = My_read,
  .write = My_write,
  .open = My_open,
  .unlocked_ioctl = My_ioctl,
  .fasync = My_fasync,
  .mmap = fopsmmap,
  .release = my_release
};
 struct vm_operations_struct  MyVmOperations = {
    .open = mymap_open,
    .close = mymap_close,
    .fault = mymap_fault,
   // .nopage = mymap_nopage
};

struct mmap_info {
	char *data;
	int reference;
};

static int hello_init(void)
{
    int major,minor=0, err;
    
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

    unregister_chrdev_region(dev, 1);
 }

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     printk(KERN_ALERT "Hello I am at Read Function \n ");
    
     return 0;
}

ssize_t My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos)
//ssize_t My_write(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     char Kernbuffer[100];
     printk(KERN_ALERT "Hello I am at Write Function \n");
     copy_from_user( Kernbuffer,buf,count);
     printk(KERN_ALERT "Got %s from user \n", Kernbuffer);
     return 0;
}

 long My_ioctl(struct file *filp, unsigned int  Cmd,  unsigned long DrvArg){

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

      printk(KERN_ALERT "Recieved on IOCTL Bytes  %lu \n",nUser);

 return 0;

}

int  My_open(struct inode *inode, struct file *filep)
{
      printk(KERN_ALERT "Hello Openning Device .........\n ");
      return 0;
}

void mymap_open(struct vm_area_struct *vma){

   printk(KERN_ALERT "In mymap_open...vm_start 0x%x,vm_end 0x%x,vm_pgoff 0x%x ,vm_page_offset 0x%x \n ",vma->vm_start,vma->vm_end,vma->vm_pgoff ,vma->vm_page_prot);
}

void mymap_close(struct vm_area_struct *vma){


}
vm_fault_t mymap_fault(struct vm_fault *vmf){
//int mymap_fault(struct vm_area_struct *vma, struct vm_fault *vmf){
 

 struct mmap_info info;
 struct page *page;
 int i;

 
#ifdef PAGE_FAULT_OLD
printk(KERN_ALERT "In mymap_fault  OLD  NO PAGE  an fault Generated .........\n ");
struct page *pageptr;
unsigned long offset = vma->vm_pgoff << PAGE_SHIFT;
unsigned long physaddr = vmf->virtual_address - vma->vm_start + offset;
unsigned long pfn = physaddr >> PAGE_SHIFT;
unsigned long virtualaddr;
printk("%s vmastart:%x vmaend:%x offset:%x physical_address:%x virtual_address: %x pfn: %x\n",
__FUNCTION__, vma->vm_start, vma->vm_end,offset, physaddr,  vmf->virtual_address,pfn);

if (!pfn_valid(pfn))
    {  
        printk(KERN_ALERT "PFN not valid" );
        return NULL;
    }

 //pageptr = virt_to_page(get_zeroed_page(GFP_KERNEL));
pageptr = pfn_to_page (pfn);
get_page(pageptr);
vmf->page = pageptr;
#else

printk(KERN_ALERT "In mymap_fault   NEW PAGE FAULT Generated .........\n ");
  info.data = (char *)get_zeroed_page(GFP_KERNEL);
    
  page = virt_to_page(info.data);
  get_page(page);
  vmf->page = page;

  for(i=0;i<4096;i++)
    info.data[i] = 0xab;
#endif

   return 0;
}


int mymap_nopage(struct vm_area_struct *vma,unsigned long address, int *type) {

    return 0;
}
 /* When the user call mmap (fd is set to this driver - /dev/MyTempDev) the function resgistered in fops (fopsmmap) will be callled.
  * This drive has option to implement fopsmmap in two ways. 
  * 1. One way is to call remap_pfn_range with the vma parameter passed to this. remap_pfn_range will create the page table entries and virtual 
  * mapping. It returns the mapped virtual address 
  * 
  * 2. Other way is to comment remap_pfn_range implementation and register MyVmOperations to the vm_operations_struct. This will trigger fault = mymap_fault
  *  
  * */
int fopsmmap(struct file *filep,struct vm_area_struct *vma) {

 printk(KERN_ALERT "Hello In fopsmmap  calll ..vm_start 0x%x,vm_end 0x%x,vm_pgoff 0x%x ,vm_page_prot 0x%x \n ",vma->vm_start,vma->vm_end,vma->vm_pgoff ,vma->vm_page_prot);

/*   if(remap_pfn_range(vma,vma->vm_start,vma->vm_pgoff,vma->vm_end - vma->vm_start,vma->vm_page_prot))
      {
          printk(KERN_EMERG "Unable to map memory ... \n");
          -EAGAIN;
       }
       */
     
 vma->vm_ops = &MyVmOperations;
 mymap_open(vma); 
  return 0;
}

int My_fasync(int fd, struct file *filep, int mode) {

 return 0;
 
}

int my_release(struct inode *inode, struct file *fil) {
  
  printk(KERN_ALERT "Hello Releaseing the Device .........\n ");
  return 0;
  
}

module_init(hello_init);
module_exit(hello_exit);
