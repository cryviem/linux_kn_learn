#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/device/class.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mod_devicetable.h>
#include "choen_common.h"

#define NUM_OF_DEVICES      2
#define MIN(x, y)           ((x < y)? x:y)
/* choen driver handler is static and hold general driver information */
struct choen_drv_handler {
    int major;
    int dev_count;
    struct class *pclass;
};

/* choen device handler is dynamic allocated upon a device appears, 
hold specific info to handle this device */
struct choen_dev_handler {
    struct choen_dev_priv *pdev_priv_data;
    dev_t dev_num;
    struct device *pdev;
    struct cdev cdev;
};

static struct choen_drv_handler choen_drv_box = {0, 0, NULL};

/****************** open - close - read - write ***********************/
static int choen_open(struct inode * p_inote, struct file * p_file)
{
    struct choen_dev_handler* p_choen_dev;
    p_choen_dev = container_of(p_inote->i_cdev, struct choen_dev_handler, cdev);
    if (p_choen_dev == NULL)
    {
        pr_err("choen_open > fail to get choen_dev_handler object\n");
        return -EFAULT;
    }
    pr_info("choen_open > device num %d - %d\n", MAJOR(p_choen_dev->dev_num), MINOR(p_choen_dev->dev_num));
    p_file->private_data = p_choen_dev;
    return 0;
}

static int choen_close(struct inode* p_inote, struct file* p_file)
{
    struct choen_dev_handler* p_choen_dev;
    p_choen_dev = p_file->private_data;
    if (p_choen_dev == NULL)
    {
        pr_err("choen_close > fail to get choen_dev_handler object\n");
        return -EFAULT;
    }
    pr_info("choen_close > device num %d - %d\n", MAJOR(p_choen_dev->dev_num), MINOR(p_choen_dev->dev_num));
    return 0;
}

static ssize_t choen_read(struct file* p_file, char __user * p_buf, size_t len, loff_t* p_offset)
{
    loff_t l_offset = *p_offset;
    struct choen_dev_handler* p_choen_dev = p_file->private_data;
    size_t l_len = MIN(strlen(p_choen_dev->pdev_priv_data->serial), len);
    
    if (l_offset != 0)
    {
        return 0;
    }

    if(0 != copy_to_user(p_buf, p_choen_dev->pdev_priv_data->serial, l_len))
    {
        return -EFAULT;
    }
    l_offset += l_len;
    *p_offset = l_offset;
    return l_len;
}
	
static ssize_t choen_write(struct file* p_file, const char __user * p_buf, size_t len, loff_t* p_offset)
{
    return len;
}

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .open = choen_open,
    .release = choen_close,
    .read = choen_read,
    .write = choen_write,
};
/****************** open - close - read - write ***********************/
int choen_probe(struct platform_device* pdev)
{
    int ret;
    struct choen_dev_priv *pdev_prv_data;
    struct choen_dev_handler *pchoen_dev_box;

    pr_info("choen_probe is called\n");
    /* get private data from device */
    pdev_prv_data = (struct choen_dev_priv*)dev_get_platdata(&pdev->dev);
    if (pdev_prv_data == NULL)
    {
        pr_warn("choen_probe > no device private data found\n");
        ret = -EINVAL;
        goto error0;
    }

    pr_info("choen_probe > connected to device serial: %s\n", pdev_prv_data->serial);
    pr_info("choen_probe > driver id is: %s - index: %ld\n", pdev->id_entry->name, pdev->id_entry->driver_data);
    /* 1 - allocate choen_dev_handler instant */
    pchoen_dev_box = kzalloc(sizeof(struct choen_dev_handler), GFP_KERNEL);
    if (!pchoen_dev_box) {
        pr_err("choen_probe > fail to allocate memory\n");
        ret = -ENOMEM;
        goto error0;
    }
    /* 2 - register character device */
    cdev_init(&pchoen_dev_box->cdev, &fops);
    pchoen_dev_box->cdev.owner = THIS_MODULE;
    pchoen_dev_box->dev_num = MKDEV(choen_drv_box.major, choen_drv_box.dev_count);
    pchoen_dev_box->pdev_priv_data = pdev_prv_data;
    ret = cdev_add(&pchoen_dev_box->cdev, pchoen_dev_box->dev_num, 1);
    if (0 != ret)
    {
        pr_err("_dev_init > fail to register cdev\n");
		goto error1;
    }
    /* 3 - create device item in /sys/class/choen-class */
    pchoen_dev_box->pdev = device_create(choen_drv_box.pclass, &pdev->dev, pchoen_dev_box->dev_num, NULL, "choen%d", choen_drv_box.dev_count);
    if (IS_ERR(pchoen_dev_box->pdev))
    {
        ret = (int)PTR_ERR(pchoen_dev_box->pdev);
        pr_warn("choen_init > fail to add device to class, err %d\n", ret);

        goto error2;
    }

    /* 4 - save choen_dev_handler instance pointer to */
    dev_set_drvdata(&pdev->dev, pchoen_dev_box);

    choen_drv_box.dev_count++;
    return 0;

error2:
    cdev_del(&pchoen_dev_box->cdev);
error1:
    kfree(pchoen_dev_box);
error0:
    return ret;
}

int choen_remove(struct platform_device* pdev)
{
    struct choen_dev_priv *pdev_prv_data;
    struct choen_dev_handler *pchoen_dev_box;
    pr_info("choen_remove is called\n");

    pdev_prv_data = (struct choen_dev_priv*)dev_get_platdata(&pdev->dev);
    if (pdev_prv_data != NULL)
    {
        pr_info("choen_remove > disconnecting from device serial: %s\n", pdev_prv_data->serial);
    }
    pchoen_dev_box = dev_get_drvdata(&pdev->dev);
    if (pchoen_dev_box == NULL)
    {
        pr_err("choen_remove > choen_dev_handler object not found\n");
        return -1;
    }
    device_destroy(choen_drv_box.pclass, pchoen_dev_box->dev_num);
    
    return 0;
}


struct platform_device_id supported_dev_id[2] = 
{
    {"choen-dev-0", 0},
    {"choen-dev-1", 1}
};

struct platform_driver choen_driver =
{
    .probe = choen_probe,
    .remove = choen_remove,
    .driver = {.name = "choen-dev"},
    .id_table = supported_dev_id
};

static int __init choen_init(void) /* Constructor */
{
    dev_t devn;
    /* 1 - register device number */
    if (0 != alloc_chrdev_region(&devn, 0, NUM_OF_DEVICES, "choen-dev-num"))
    {
        pr_warn("choen_init > fail to allocate device number\n");
		goto error1;
    }
    choen_drv_box.major = MAJOR(devn);
    /* 2 - create class */
    choen_drv_box.pclass = class_create(THIS_MODULE, "choen-class");
    if (IS_ERR(choen_drv_box.pclass))
    {
        pr_warn("choen_init > fail to create class, err %ld\n", PTR_ERR(choen_drv_box.pclass));
        goto error2;
    }
    /* 3 - register platform driver */
    if (0 != platform_driver_register(&choen_driver))
    {
        pr_warn("choen_init > fail to register platform device\n");
        goto error3;
    }

    pr_info("Hello: choen driver loaded\n");
    
    return 0;

error3:
    class_destroy(choen_drv_box.pclass);
error2:
    unregister_chrdev_region(devn, NUM_OF_DEVICES);
error1:
    return -1;
}

static void __exit choen_exit(void) /* Destructor */
{
    pr_info("Goodbye: choen driver unloaded\n");
    platform_driver_unregister(&choen_driver);
    class_destroy(choen_drv_box.pclass);
    unregister_chrdev_region(MKDEV(choen_drv_box.major, 0), NUM_OF_DEVICES);
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_INFO(intree, "Y");
