#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/list.h>
#include <linux/slab.h> 
#include <linux/sysfs.h> 
#include <linux/kobject.h>

MODULE_LICENSE("Dual BSD/GPL");

static char   message[256] = {0};

#define _GET_DATA   _IOR('M', 1, unsigned long * )
#define _PUT_DATA    _IOR('M', 2, unsigned long * )

#define End_Spawap32(num)        ( ( (num >> 24)&0xff) |   (( (num >> 16)&0xff) << 8) |   (( ( num >> 8)&0xff) << 16) |  (( ( num)&0xff) << 24) )

#define NosEnd_Spawap32(num)        (  (num >> 24) |    (num >> 16) |   ( num >> 8) |  ( num && 0xff000000))

struct list_head queue_list_pack;
LIST_HEAD(queue_list_pack);
      

int extarct_packet(void);
ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
//ssize_t  My_write(struct file *filep, char __user *buf,size_t count,loff_t *ppos);
ssize_t    My_write(struct file *filep,const char __user *buf,size_t count,loff_t *ppos);
int  My_open(struct inode *, struct file *);
//static long My_ioctl(struct file *, unsigned int , unsigned int);
static long My_ioctl(struct file *, unsigned int , unsigned long);
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
  .owner = THIS_MODULE,
  .read = My_read,
  .write = My_write,
  .open = My_open,
  .unlocked_ioctl = My_ioctl,
  .fasync = My_fasync
};

  dev_t dev;
  int gQDepth = 125;
  static struct class *radar_class = NULL;
  struct device *device = NULL;
  volatile int etx_value = 0;
  struct kobject *kobj_ref;
  struct cdev *MyCDev;
  
  static ssize_t func_gpio_capture_show(struct device *dev, struct device_attribute *attr, char *buf);
  static ssize_t func_gpio_capture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) ;
  
  static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf);
  static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count); 

  static DEVICE_ATTR(func_gpio_capture,S_IWUSR | S_IRUGO,func_gpio_capture_show,func_gpio_capture_store);
  
  struct kobj_attribute ext_attrb = __ATTR(ext_value, 0770, sysfs_show, sysfs_store); 
  
static int hello_init(void)
{
    int major,minor=0, err;
    struct MyData sd;
    ktime_t kerTime;
    

    radar_class = class_create(THIS_MODULE, "DemoSysfsClass");
    
    alloc_chrdev_region(&dev,minor,1,"MyTempDev");

    MyCDev = cdev_alloc();
    MyCDev->ops = &MyTempDevfops;
     
    major = MAJOR(dev);
    minor = MINOR(dev);

    printk(KERN_ALERT "Hello Welcome to MyTempDrv  MajNum: %d  ,  MinNum%d \n ",major,minor);
        
    devnum = MKDEV(major,minor);
    err = cdev_add(MyCDev,devnum,1);
   
    if (err)
      printk(KERN_ALERT " Errrrror "); 
      
    device = device_create(radar_class, NULL, dev, NULL, "stm32-adc%d", (minor));   
      
    err = device_create_file(device, &dev_attr_func_gpio_capture);
	
    if (err < 0) {
			printk(KERN_ERR "Cant create device attribute func_gpio_capture\n");
	   }
   
    kobj_ref = kobject_create_and_add("etx_sysfs",kernel_kobj);
    
     if(sysfs_create_file(kobj_ref,&ext_attrb.attr)){
                pr_err("Cannot create sysfs file......\n");
                goto r_sysfs;
    }
    
    kerTime = ktime_get();
    
    printk( KERN_ALERT  "No Cdev Add Error ...Sucess !!!!! Ktime is %d\n",kerTime);
    
    return 0;
    
    r_sysfs:
            kobject_put(kobj_ref); 
            sysfs_remove_file(kernel_kobj, &ext_attrb.attr);
    return -1;
   
}

static void hello_exit(void)
{
    printk(KERN_ALERT "Hello Goodbye, Driver world \n");

    unregister_chrdev_region(dev, 1);
    
        kobject_put(kobj_ref); 
        sysfs_remove_file(kernel_kobj, &ext_attrb.attr);
        device_destroy(radar_class,dev);
        class_destroy(radar_class);
        cdev_del(MyCDev);
      
        pr_info("Device Driver Remove...Done!!!\n");
 }

ssize_t  My_read(struct file *filep, char __user *buf,size_t count,loff_t *ppos)
{
     long int i=0;
     printk(KERN_ALERT "Hello I am at Read Function \n ");
    
     return 0;
}


ssize_t My_write(struct file *filep, const char __user *buf,size_t count,loff_t *ppos)
{
     int size_of_message;
     printk(KERN_ALERT "Hello I am at Write Function \n");
     
     return count;
}

static long My_ioctl(struct file *filp, unsigned int  Cmd,  unsigned long  DrvArg){

  unsigned long  nUser;

  printk(KERN_ALERT "Hello I am at IOCTL Function  \n");

 switch (Cmd)
    {
        /* Get   request packets */
        case _GET_DATA:
            
                if (copy_to_user(( unsigned long * )DrvArg, &gQDepth, 1))
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
      
      return 0;

}



static ssize_t func_gpio_capture_show(struct device *dev, struct device_attribute *attr, char *buf){

  int ret;
  uint32_t value;
 
  // Gpio_Set_t gpioData;
  //uint8_t pin = gpioReadPin ;

  printk(KERN_ALERT"Inside the function %s \n",__func__);
  //printk(KERN_ALERT"Recieved from User the pin  %d \n",pin);
  //Gpio_Config_Get(&value,pin);
  //printk(KERN_ALERT"Read value from the Pin %d is %d \n",pin,value);

  sprintf(buf,"%d",value);
  printk(KERN_EMERG"In func_gpio_capture_show  value from the capture_show is %d \n",value);
  
  return 0;

}

static ssize_t func_gpio_capture_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {

    int ret;
    uint32_t value;
    uint8_t pin;
   
    printk(KERN_ALERT"Inside function %s\n",__func__);

    ret = sscanf(buf, "%d %d", &pin, &value);

    //printk(KERN_ALERT"Got GPIO pin %d to set val %d From user \n",pin,value);
    
    //Gpio_Config_Set(value, pin);  
    
    printk(KERN_EMERG"In func_gpio_capture_store printing values from capture_store is Pin  %d and Value %d and count is %d\n",pin ,value,count);  
    
    return count;   

}

static ssize_t sysfs_show(struct kobject *kobj,struct kobj_attribute *attr,char *buf)
{
  
  return 0;
}

static ssize_t sysfs_store(struct kobject *kobj,struct kobj_attribute *attr,const char *buf,size_t count)
{
  
 return count;
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
