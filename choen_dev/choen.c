#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/fs.h>   /* for handle device number */
#include <linux/cdev.h> /* for handle cdev struct */
#include <linux/ioctl.h>
#include <asm/uaccess.h>
#include "choen.h"

#define NUM_OF_DEVICES               2
#define RW_BUFF_SIZE                 256
struct choen_dev_t {
    const char* name;
    struct cdev cdev;
    int ioctl_test_buff;
    char rw_test_buff[RW_BUFF_SIZE];
};

static struct choen_dev_t dev_table[NUM_OF_DEVICES];
static dev_t dev_num;

static int choen_open(struct inode * p_inote, struct file * p_file)
{
    struct choen_dev_t* p_dev;
    printk(KERN_INFO "choen_open > is called\n");
    p_dev = container_of(p_inote->i_cdev, struct choen_dev_t, cdev);
    if (p_dev == NULL)
    {
        printk(KERN_WARNING "choen_open > fail to get choen_dev_t object\n");
        return -EFAULT;
    }
    printk(KERN_INFO "choen_open > device is %s\n", p_dev->name);
    p_file->private_data = p_dev;
    return 0;
}

static int choen_close(struct inode* p_inote, struct file* p_file)
{
    struct choen_dev_t* p_dev = p_file->private_data;
    printk(KERN_INFO "choen_close > is called\n");
    if (p_dev == NULL)
    {
        printk(KERN_WARNING "choen_close > fail to get choen_dev_t object\n");
        return -EFAULT;
    }
    printk(KERN_INFO "choen_close > device is %s\n", p_dev->name);
    return 0;
}

static ssize_t choen_read(struct file* p_file, char __user * p_buf, size_t len, loff_t* p_offset)
{
    printk(KERN_INFO "choen_read > is called\n");
    return 0;
}
	
static ssize_t choen_write(struct file* p_file, const char __user * p_buf, size_t len, loff_t* p_offset)
{
    printk(KERN_INFO "choen_write > is called\n");
    /* return len to assume write operation successfully handled */
    return len;
}

static long choen_ioctl(struct file *p_file, unsigned int cmd, unsigned long arg)
{
    int ret;
    struct choen_dev_t* p_dev = p_file->private_data;

    printk(KERN_INFO "choen_ioctl > is called\n");

    if(_IOC_TYPE(cmd) != CHOEN_MAGIC_NUMBER)
    {
        printk(KERN_WARNING "choen_ioctl > Command is invalid\n");
        return -ENOTTY;
    }

    if (_IOC_DIR(cmd) == _IOC_READ)
        ret = access_ok((void __user *)arg, _IOC_SIZE(cmd));
    else 
        ret = access_ok((void __user *)arg, _IOC_SIZE(cmd));

    if (ret == 0)
    {
        printk(KERN_WARNING "choen_ioctl > Bad param\n");
        return -EFAULT;
    }

    switch (cmd)
    {
        case IOCTL_CMD_HELLO:
        printk(KERN_INFO "choen_ioctl > IOCTL_CMD_HELLO\n");
        break;

        case IOCTL_CMD_SET_VALUE:
        printk(KERN_INFO "choen_ioctl > IOCTL_CMD_SET_VALUE\n");
        p_dev->ioctl_test_buff = arg;
        break;

        case IOCTL_CMD_SET_PTR:
        printk(KERN_INFO "choen_ioctl > IOCTL_CMD_SET_PTR\n");
        __get_user(p_dev->ioctl_test_buff, (int __user *)arg);
        break;

        case IOCTL_CMD_GET_PTR:
        printk(KERN_INFO "choen_ioctl > IOCTL_CMD_GET_PTR\n");
        __put_user(p_dev->ioctl_test_buff, (int __user *)arg);
        break;

        default:
        printk(KERN_WARNING "choen_ioctl > CMD UNSUPPORTED\n");
        return -ENOTTY;
        break;
    }
    return 0;
}
static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = choen_open,
    .release = choen_close,
    .read = choen_read,
    .write = choen_write,
    .unlocked_ioctl = choen_ioctl,
};

static int _dev_init(int index, const char* dev_name, int supported_minors)
{
    int major_num = MAJOR(dev_num);
    dev_t l_dev_num = MKDEV(major_num, index);

    cdev_init(&dev_table[index].cdev, &fops);
    dev_table[index].cdev.owner = THIS_MODULE;
    dev_table[index].name = dev_name;

    if (0 != cdev_add(&dev_table[index].cdev, l_dev_num, supported_minors))
    {
        printk(KERN_WARNING "_dev_init > fail to register cdev index %d\n", index);
		return -1;
    }
    printk(KERN_INFO "_dev_init > SUCCESS !!!, major %d - minor %d\n", major_num, index);
    return 0;
}

static int __init choen_init(void) /* Constructor */
{
    printk(KERN_INFO "Hello: choen registered\n");

    if (0 != alloc_chrdev_region(&dev_num, 0, NUM_OF_DEVICES, "choen"))
    {
        printk(KERN_WARNING "choen_init > fail to allocate device number\n");
		goto error0;
    }

    if ( _dev_init(0, "dev0", 1) || _dev_init(1, "dev1", 1))
    {
        goto error1;
    }

    return 0;

error1:
    unregister_chrdev_region(dev_num, NUM_OF_DEVICES);
error0:
    return -1;
}

static void __exit choen_exit(void) /* Destructor */
{
    printk(KERN_INFO "Goodbye: choen unregistered\n");

    cdev_del(&dev_table[0].cdev);
    cdev_del(&dev_table[1].cdev);
    unregister_chrdev_region(dev_num, NUM_OF_DEVICES);
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Hello world ldd");
MODULE_INFO(intree, "Y");
