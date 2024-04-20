#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/fs.h> 
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/device/class.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mod_devicetable.h>

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
    dev_t dev_num;
    struct device *pdev;
};

static struct choen_drv_handler choen_drv_box = {0, 0, NULL};

int choen_probe(struct platform_device* pdev)
{
    int ret;
    struct choen_dev_handler *pchoen_dev_box;

    pr_info("choen_probe is called\n");

    /* 1 - allocate choen_dev_handler instant */
    pchoen_dev_box = kzalloc(sizeof(struct choen_dev_handler), GFP_KERNEL);
    if (!pchoen_dev_box) {
        pr_err("choen_probe > fail to allocate memory\n");
        ret = -ENOMEM;
        goto error0;
    }
    pchoen_dev_box->dev_num = MKDEV(choen_drv_box.major, choen_drv_box.dev_count);
    /* 2 - create device item in /sys/class/choen-class */
    pchoen_dev_box->pdev = device_create(choen_drv_box.pclass, &pdev->dev, pchoen_dev_box->dev_num, NULL, "choen%d", choen_drv_box.dev_count);
    if (IS_ERR(pchoen_dev_box->pdev))
    {
        ret = (int)PTR_ERR(pchoen_dev_box->pdev);
        pr_warn("choen_probe > fail to add device to class, err %d\n", ret);
        goto error1;
    }

    /* 3 - save choen_dev_handler instance pointer to */
    dev_set_drvdata(&pdev->dev, pchoen_dev_box);  
    choen_drv_box.dev_count++;

    return 0;

error1:
    kfree(pchoen_dev_box);
error0:
    return ret;
}

int choen_remove(struct platform_device* pdev)
{
    struct choen_dev_handler *pchoen_dev_box;

    pr_info("choen_remove is called\n");

    pchoen_dev_box = dev_get_drvdata(&pdev->dev);
    if (pchoen_dev_box == NULL)
    {
        pr_err("choen_remove > choen_dev_handler object not found\n");
        return -1;
    }

    device_destroy(choen_drv_box.pclass, pchoen_dev_box->dev_num);
    kfree(pchoen_dev_box);
    return 0;
}



static struct of_device_id matching_table[] =
{
    {.compatible = "org,choen-gpio"},
    {}
};

static struct platform_driver choen_driver =
{
    .probe = choen_probe,
    .remove = choen_remove,
    .driver = {.name = "choen-dev", .of_match_table = matching_table},
};

static int __init choen_init(void) /* Constructor */
{
    int ret;
    dev_t devn;
    /* 1 - register device number */
    if (0 != alloc_chrdev_region(&devn, 0, NUM_OF_DEVICES, "choen-dev-num"))
    {
        pr_warn("choen_init > fail to allocate device number\n");
		goto error1;
    }
    choen_drv_box.major = MAJOR(devn);
    /* 1 - create class */
    choen_drv_box.pclass = class_create(THIS_MODULE, "choen-class");
    if (IS_ERR(choen_drv_box.pclass))
    {
        ret = (int)PTR_ERR(choen_drv_box.pclass);
        pr_warn("choen_init > fail to create class, err %d\n", ret);
        goto error2;
    }
    /* 2 - register platform driver */
    ret = platform_driver_register(&choen_driver);
    if (0 != ret)
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
    return ret;
}

static void __exit choen_exit(void) /* Destructor */
{
    platform_driver_unregister(&choen_driver);
    class_destroy(choen_drv_box.pclass);
    pr_info("Goodbye: choen driver unloaded\n");
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("BBB GPIO custom driver");
MODULE_INFO(intree, "Y");
