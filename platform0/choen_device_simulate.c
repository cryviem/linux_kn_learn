/*
This module aim to register those pseudo platform devices
*/


#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>

/* let make some private data for device */
struct choen_dev_priv {
    const char * serial;
};
struct choen_dev_priv choen_priv_table[2] = 
{
    {.serial = "CHOENSN0"},
    {.serial = "CHOENSN1"}
};

/* this release function aim to free up those private data if any*/
void choen_dev_release(struct device* pdev)
{
    printk(KERN_INFO "choen_dev_release > should clean up those private data\n");
}

/* create this struct to carry all devices information */
struct platform_device choen_dev0 = 
{
    .name = "choen-dev",
    .id = 0,
    .dev = {.platform_data = &choen_priv_table[0], .release = choen_dev_release}
};

struct platform_device choen_dev1 = 
{
    .name = "choen-dev",
    .id = 1,
    .dev = {.platform_data = &choen_priv_table[1], .release = choen_dev_release}
};

static int __init choen_init(void) /* Constructor */
{
    /* register platform device to kernel */
    platform_device_register(&choen_dev0);
    platform_device_register(&choen_dev1);
    printk(KERN_INFO "Hello: choen device simulation loaded\n");

    return 0;
}

static void __exit choen_exit(void) /* Destructor */
{
    /* do reverse, unregister*/
    platform_device_unregister(&choen_dev0);
    platform_device_unregister(&choen_dev1);
    printk(KERN_INFO "Goodbye: choen device simulation unloaded\n");
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Hello world ldd");
MODULE_INFO(intree, "Y");
