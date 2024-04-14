#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h> 
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/device/class.h>
#include "choen_common.h"

#define NUM_OF_DEVICES      2

struct choen_drv_handler {
    dev_t dev_num_base;
    struct class *pclass;
};

struct choen_dev_handler {
    struct cdev cdev;
};

static struct choen_drv_handler choen_drv_box;

int choen_probe(struct platform_device* pdev)
{
    struct choen_dev_priv *pdev_prv_data;
    struct choen_dev_handler *pchoen_dev_box;

    pr_info("choen_probe is called\n");
    /* get private data from device */
    pdev_prv_data = (struct choen_dev_priv*)dev_get_platdata(&pdev->dev);
    if (pdev_prv_data == NULL)
    {
        pr_warn("choen_probe > no device private data found\n");
        return -EINVAL;
    }

    pr_info("choen_probe > connected to device serial: %s\n", pdev_prv_data->serial);

    /* 1 - allocate choen_dev_handler instant */
    /* 2 - register character device */
    /* 3 - create device item in /sys/class/choen-class */
    return 0;
}

int choen_remove(struct platform_device* pdev)
{
    struct choen_dev_priv *pdev_prv_data;

    pr_info("choen_remove is called\n");

    pdev_prv_data = (struct choen_dev_priv*)dev_get_platdata(&pdev->dev);
    if (pdev_prv_data != NULL)
    {
        pr_info("choen_probe > disconnecting from device serial: %s\n", pdev_prv_data->serial);
    }
    
    return 0;
}

struct platform_driver choen_driver =
{
    .probe = choen_probe,
    .remove = choen_remove,
    .driver = {.name = "choen-dev"}
};

static int __init choen_init(void) /* Constructor */
{
    /* 1 - register device number */
    if (0 != alloc_chrdev_region(&choen_drv_box.dev_num_base, 0, NUM_OF_DEVICES, "choen-dev-num"))
    {
        pr_warn("choen_init > fail to allocate device number\n");
		goto error1;
    }

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
    unregister_chrdev_region(choen_drv_box.dev_num_base, NUM_OF_DEVICES);
error1:
    return -1;
}

static void __exit choen_exit(void) /* Destructor */
{
    pr_info("Goodbye: choen driver unloaded\n");
    platform_driver_unregister(&choen_driver);
    class_destroy(choen_drv_box.pclass);
    unregister_chrdev_region(choen_drv_box.dev_num_base, NUM_OF_DEVICES);
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_INFO(intree, "Y");
