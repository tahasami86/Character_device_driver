#include <linux/module.h>
#include <linux/init.h>
#include <linux/fcntl.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/cdev.h>
#include <linux/fs.h>

MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("Taha sami <tahasami8@gmail.com>");
MODULE_DESCRIPTION("Kernel character driver for Commuincation with DPDK Application");

#define DEVICE_NAME "dpdk_dev"
#define CLASS_NAME "dpdk_dev_class"
#define IOCTL_MAGIC 'd'

#define SEND_DATA_VALUE _IOW(IOCTL_MAGIC,'a',struct my_data)
#define SEND_READ_VALUE _IOR(IOCTL_MAGIC,'b',struct my_data)

struct my_data
{
    /* data */
    u_int32_t data;
    char message[100];
};

static struct my_data kernel_data;
static dev_t dev = 0;
static struct class *dev_class;
static struct cdev dev_cdev;

/*
** This function will be called when we open the Device file
*/
static int ext_open(struct inode *inode, struct file *file)
{
        pr_info("Driver Open Function Called...!!!\n");
        pr_info("MAJOR_NUMBER : %d \n ",imajor(inode));
        pr_info("MINOR_NUMBER : %d \n ",iminor(inode));
        return 0;
}
/*
** This function will be called when we close the Device file
*/
static int ext_release(struct inode *inode, struct file *file)
{
        pr_info("Driver Release Function Called...!!!\n");
        return 0;
}

/*

This function will be called when reading from the device file

*/
static ssize_t ext_read(struct file *filp, char __user *buf, size_t len, loff_t *off) {

        pr_info("Driver Read Function Called...!!!\n");
        return 0;
}

/*

This function will be called when reading from the device file
*/

static ssize_t ext_write(struct file *filp,const char *buf, size_t len, loff_t *off) {

        pr_info("Driver Write Function Called...!!!\n");
        return len;
}


static long ext_ioctl (struct file *filp, unsigned int cmd, unsigned long arg) {

        //pr_info("Driver Ioctl Function Called...!!!\n");

        switch(cmd) {
                case SEND_DATA_VALUE:
                /*copy data from user space to kernel space*/
                if(copy_from_user(&kernel_data,(struct my_data *)arg,sizeof(struct my_data))) {
                        pr_err("copy_from_user failed \n");
                        return -EFAULT;
                }
                pr_info("Data recevied from user \n");
                pr_info("Data : %d \n",kernel_data.data);
                pr_info("Message : %s \n",kernel_data.message);
                break;
                case SEND_READ_VALUE:
                kernel_data.data += 10;  
                strncpy(kernel_data.message,"Hello, This message is from kernel",sizeof(kernel_data.message));

                /*copy data from kernel space to user space*/
                if(copy_to_user((struct my_data *)arg,&kernel_data,sizeof(struct my_data))) {
                        pr_err("copy_to_user failed \n");
                        return -EFAULT;
                }
                pr_info("Data send to user \n");
                pr_info("Data : %d \n",kernel_data.data);
                pr_info("Message : %s \n",kernel_data.message);
                break;
                default:
                pr_err("Invalid Command \n");
                return -EINVAL;
        }

        return 0;
}
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = ext_open,
    .release = ext_release,
    .read = ext_read,
    .write = ext_write,
    .unlocked_ioctl = ext_ioctl
};

static int __init my_init(void) {

        if(alloc_chrdev_region(&dev,0,1,DEVICE_NAME) < 0) {
            pr_err("alloc_chrdev_region failed \n");
            return -1;
        }

        cdev_init(&dev_cdev,&fops);
        if ((cdev_add(&dev_cdev,dev,1)) < 0) {
            pr_err("cdev_add failed \n");
            goto err_class_create;
        }
        /*Creating struct class*/
        dev_class = class_create(THIS_MODULE,CLASS_NAME);
        if(IS_ERR(dev_class)) {
            pr_err("class_create failed \n");
            goto err_class_create;
        }

         /*Creating device*/
         if(IS_ERR(device_create(dev_class,NULL,dev,NULL,DEVICE_NAME))) {
            pr_err("device_create failed \n");
            goto err_device_create;
         }
        
       
        pr_info("Kernel Module Inserted Successfully...\n");
        return 0;


err_device_create:
        class_destroy(dev_class);
err_class_create:
        unregister_chrdev_region(dev,1);
    return 0;
}

static void __exit my_exit(void) {
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&dev_cdev);
    unregister_chrdev_region(dev,1);
    pr_info("Kernel Module Removed Successfully...\n");
}
module_init(my_init)
module_exit(my_exit)
