#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>


int choen_probe(struct platform_device* pdev)
{
    printk(KERN_INFO "choen_probe is called\n");
    return 0;
}

int choen_remove(struct platform_device* pdev)
{
    printk(KERN_INFO "choen_remove is called\n");
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
    printk(KERN_INFO "Hello: choen driver loaded\n");
    platform_driver_register(&choen_driver);
    return 0;
}

static void __exit choen_exit(void) /* Destructor */
{
    printk(KERN_INFO "Goodbye: choen driver unloaded\n");
    platform_driver_unregister(&choen_driver);
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Hello world ldd");
MODULE_INFO(intree, "Y");
