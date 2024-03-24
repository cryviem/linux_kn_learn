#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>


static int __init choen_init(void) /* Constructor */
{
    printk(KERN_INFO "Hello: choen driver loaded\n");

    return 0;
}

static void __exit choen_exit(void) /* Destructor */
{
    printk(KERN_INFO "Goodbye: choen driver unloaded\n");
}

module_init(choen_init);
module_exit(choen_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Nguyen Hong An");
MODULE_DESCRIPTION("Hello world ldd");
MODULE_INFO(intree, "Y");
