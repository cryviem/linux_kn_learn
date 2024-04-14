#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/platform_device.h>
#include "choen_common.h"

int choen_probe(struct platform_device* pdev)
{
    struct choen_dev_priv *pdev_prv_data;

    pr_info("choen_probe is called\n");
    /* get private data from device */
    pdev_prv_data = (struct choen_dev_priv*)dev_get_platdata(&pdev->dev);
    if (pdev_prv_data == NULL)
    {
        pr_warn("choen_probe > no device private data found\n");
        return -EINVAL;
    }

    pr_info("choen_probe > connected to device serial: %s\n", pdev_prv_data->serial);
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
    pr_info("Hello: choen driver loaded\n");
    platform_driver_register(&choen_driver);
    return 0;
}

static void __exit choen_exit(void) /* Destructor */
{
    pr_info("Goodbye: choen driver unloaded\n");
    platform_driver_unregister(&choen_driver);
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Simple platform driver");
MODULE_INFO(intree, "Y");
