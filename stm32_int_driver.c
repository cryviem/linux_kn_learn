/* LINUX USB DEVICE DRIVER*/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/kref.h>
//#include <linux/smp_lock.h>
#include <linux/usb.h>
#include <asm/uaccess.h>

/*****MACROS-DEFINITION*******************************/
/* vendor id and product id ( must be the same with device ) */
#define STM32_VENDOR_ID					0x0483
#define STM32_PRODUCT_ID				0x5710

#define STM32_BULK_IN_EP				0x81
#define STM32_IN_EP_MAX_SIZE			4

#define STM32_BULK_OUT_EP				0x01
#define STM32_OUT_EP_MAX_SIZE			4


#define USB_STM32_MINOR_BASE			192 //?????

#define to_usb_stm32(d) container_of(d, struct usb_stm32, stm32_kref)
/*****************************************************/

static struct usb_driver stm32_usb_driver;

struct usb_stm32 {
	struct usb_device *	stm32_udev;			
	struct usb_interface *	stm32_interface;		
	
	char *			stm32_Rx_Int_Buffer;		
	char *			stm32_Tx_Int_Buffer;		
	size_t			stm32_Rx_Packet_Size;		
	size_t			stm32_Tx_Packet_Size;		
	__u8			stm32_INT_IN_EPAddr;		
	__u8			stm32_INT_OUT_EPAddr;		
	__u8			stm32_IN_EP_Interval;		
	__u8			stm32_OUT_EP_Interval;		
	char * 			stm32_Rx_UsrSpc_Buf;
	size_t			stm32_Rx_Actual_Size;
	struct kref		stm32_kref;
};


/* confirmed */
static struct usb_device_id stm32_usb_id_table [] = {
	{ USB_DEVICE(STM32_VENDOR_ID, STM32_PRODUCT_ID) },
	{ }
};
MODULE_DEVICE_TABLE (usb, stm32_usb_id_table);

static void stm32_delete(struct kref *kref)
{	
	struct usb_stm32 *dev = to_usb_stm32(kref);

	usb_put_dev(dev->stm32_udev);
	kfree (dev->stm32_Rx_Int_Buffer);
	kfree (dev->stm32_Tx_Int_Buffer);
	kfree (dev);
}

static void stm32_write_callback(struct urb *urb)
{
	printk(KERN_INFO "stm32_write_callback: urb_status %d\n", urb->status);
	
}

static void stm32_read_callback(struct urb *urb)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	printk(KERN_INFO "stm32_write_callback: urb_status %d\n", urb->status);

	pUSB_stm32_dev = (struct usb_stm32 *)urb->context;
	copy_to_user(pUSB_stm32_dev->stm32_Rx_UsrSpc_Buf, pUSB_stm32_dev->stm32_Rx_Int_Buffer, pUSB_stm32_dev->stm32_Rx_Actual_Size);
}

static ssize_t stm32_read(struct file *file, char __user *buffer, size_t count, loff_t *ppos)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	struct urb *pl_Urb = NULL;
	int l_RetVal = 0;
	
	/* get usb_stm32 struct */
	pUSB_stm32_dev = (struct usb_stm32 *)file->private_data;
	if (pUSB_stm32_dev == NULL)
	{
		printk(KERN_INFO "stm32_read: retrieve usb_stm32 FAIL\n");
		l_RetVal = -ENODEV;
		goto error0;
	}
	
	/* check size of tx packet */
	if ((count < 1) || (count > pUSB_stm32_dev->stm32_Rx_Packet_Size))
	{
		printk(KERN_INFO "stm32_read: rx packet size is invalid\n");
		goto error0;
	}
	
	/* save user space buffer */
	pUSB_stm32_dev->stm32_Rx_UsrSpc_Buf = buffer;
	pUSB_stm32_dev->stm32_Rx_Actual_Size = count;
	/* allocate a URB for transmition */
	pl_Urb = usb_alloc_urb(0, GFP_KERNEL);
	if (pl_Urb = NULL) 
	{
		printk(KERN_INFO "stm32_read: URB create FAIL\n");
		l_RetVal = -ENOMEM;
		goto error1;
	}

	/* initialize the urb properly */
	usb_fill_int_urb(pl_Urb, pUSB_stm32_dev->stm32_udev,
			  usb_rcvintpipe(pUSB_stm32_dev->stm32_udev, pUSB_stm32_dev->stm32_INT_IN_EPAddr),
			  pUSB_stm32_dev->stm32_Rx_Int_Buffer, count, stm32_read_callback, pUSB_stm32_dev, pUSB_stm32_dev->stm32_IN_EP_Interval);	
	
	l_RetVal = usb_submit_urb(pl_Urb, GFP_KERNEL);
	if (l_RetVal) 
	{
		printk(KERN_INFO "stm32_read: URB submit FAIL, ret: %d\n", l_RetVal);
		goto error1;
	}
	
error1:
	usb_free_urb(pl_Urb);
error0:
	return l_RetVal;
}

static ssize_t stm32_write(struct file *file, const char __user *user_buffer, size_t count, loff_t *ppos)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	struct urb *pl_Urb = NULL;
	int l_RetVal = 0;
	
	/* get usb_stm32 struct */
	pUSB_stm32_dev = (struct usb_stm32 *)file->private_data;
	if (pUSB_stm32_dev == NULL)
	{
		printk(KERN_INFO "stm32_write: retrieve usb_stm32 FAIL\n");
		l_RetVal = -ENODEV;
		goto error0;
	}
	
	/* check size of tx packet */
	if ((count < 1) || (count > pUSB_stm32_dev->stm32_Tx_Packet_Size))
	{
		printk(KERN_INFO "stm32_write: tx packet size is invalid\n");
		goto error0;
	}
	
	/* copy data from user space */
	if (copy_from_user(pUSB_stm32_dev->stm32_Tx_Int_Buffer, user_buffer, count)) 
	{
		l_RetVal = -EFAULT;
		goto error0;
	}
	
	/* allocate a URB for transmition */
	pl_Urb = usb_alloc_urb(0, GFP_KERNEL);
	if (pl_Urb = NULL) 
	{
		printk(KERN_INFO "stm32_write: URB create FAIL\n");
		l_RetVal = -ENOMEM;
		goto error1;
	}

	/* initialize the urb properly */
	usb_fill_int_urb(pl_Urb, pUSB_stm32_dev->stm32_udev,
			  usb_sndintpipe(pUSB_stm32_dev->stm32_udev, pUSB_stm32_dev->stm32_INT_OUT_EPAddr),
			  pUSB_stm32_dev->stm32_Tx_Int_Buffer, count, stm32_write_callback, pUSB_stm32_dev, pUSB_stm32_dev->stm32_OUT_EP_Interval);	
	
	l_RetVal = usb_submit_urb(pl_Urb, GFP_KERNEL);
	if (l_RetVal) 
	{
		printk(KERN_INFO "stm32_write: URB submit FAIL, ret: %d\n", l_RetVal);
		goto error1;
	}
	
error1:
	usb_free_urb(pl_Urb);
error0:
	return l_RetVal;
}

/* fixed:start*/
static int stm32_open(struct inode *inode, struct file *file)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	struct usb_interface *pl_interface = NULL;
	int l_minor;
	int l_RetVal = 0;
	
	printk(KERN_INFO "stm32_open is called\n");
	/* get minor number */
	l_minor = iminor(inode);
	/* search for interface */
	pl_interface = usb_find_interface(&stm32_usb_driver, l_minor);
	if (pl_interface = NULL) {
		printk(KERN_INFO "stm32_open: interface not found for minor: %d\n", l_minor);
		l_RetVal = -ENODEV;
		goto exit;
	}
	
	/* get specific usb_stm32 struct */
	pUSB_stm32_dev = usb_get_intfdata(pl_interface);
	if (pUSB_stm32_dev = NULL)
	{
		printk(KERN_INFO "stm32_open: get usb_stm32 FAIL\n");
		l_RetVal = -ENODEV;
		goto exit;
	}
	
	/* mark refer to usb_kref struct */
	/* refer at open - finish at release function */
	kref_get(&pUSB_stm32_dev->stm32_kref);
	/* save usb_kref struct to file */
	/* for write, read, release function use*/
	file->private_data = pUSB_stm32_dev;
	
	printk(KERN_INFO "stm32_open done successfully\n");
exit:
	return l_RetVal;
}
/* fixed:end*/

/* fixed:start*/
static int stm32_release(struct inode *inode, struct file *file)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	
	printk(KERN_INFO "stm32_release is called\n");
	
	pUSB_stm32_dev = (struct usb_stm32 *)file->private_data;
	if (pUSB_stm32_dev == NULL)
	{
		printk(KERN_INFO "stm32_release: retrieve usb_stm32 FAIL\n");
		return -ENODEV;
	}
	kref_put(&pUSB_stm32_dev->stm32_kref, stm32_delete);
	printk(KERN_INFO "stm32_release done successfully\n");
	return 0;
}
/* fixed:end*/

/* fixed:start*/
static struct file_operations stm32_fops = {
	.owner =	THIS_MODULE,
	.read =		stm32_read,
	.write =	stm32_write,
	.open =		stm32_open,
	.release =	stm32_release,
};
/* fixed:end*/

/* fixed:start*/
static struct usb_class_driver stm32_class = {
	.name = "usb/stm32_device%d",
	.fops = &stm32_fops,
	.minor_base = USB_STM32_MINOR_BASE,
};
/* fixed:end*/

/* fixed:start*/
/* probe function: 
   be called when device attached to PC and the usb core think that this
   driver should handle.
   probe function should perform checks on info passed to it about device 
   and decide whether the driver is really appproriate for that device.*/
static int stm32_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
	int retval = 0;
	unsigned char l_ForLoopIndex_UB;
	unsigned int l_MaxPacketSize_UW = 0;
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	struct usb_host_interface *pInterface_Descriptor;
	struct usb_endpoint_descriptor *pEndpoint_Descriptor;
	
	printk(KERN_INFO "stm32_usb: 0 - start probe function.\n");

	/* allocate usb_stm32 struct*/
	printk(KERN_INFO "stm32_usb: 1 - allocate usb_stm32 struct.\n");
	pUSB_stm32_dev = kmalloc(sizeof(struct usb_stm32), GFP_KERNEL);
	if (pUSB_stm32_dev == NULL) {
		printk(KERN_INFO "stm32_usb: 1.1 - allocate usb_stm32 struct FAIL.\n");
		goto error;
	}
	printk(KERN_INFO "stm32_usb: 2 - allocate usb_stm32 struct DONE.\n");
	
	/* initialize value for usb_stm32 struct*/
	memset(pUSB_stm32_dev, 0x00, sizeof (*pUSB_stm32_dev));
	/* kref initializing*/
	kref_init(&pUSB_stm32_dev->stm32_kref);
	
	/* get usb_device struct and usb_interface struct */
	pUSB_stm32_dev->stm32_udev = usb_get_dev(interface_to_usbdev(interface));
	pUSB_stm32_dev->stm32_interface = interface;

	/* get endpoint automatically */
	pInterface_Descriptor = interface->cur_altsetting;
	
	for (l_ForLoopIndex_UB = 0; l_ForLoopIndex_UB < pInterface_Descriptor->desc.bNumEndpoints; ++l_ForLoopIndex_UB) 
	{
		pEndpoint_Descriptor = &pInterface_Descriptor->endpoint[l_ForLoopIndex_UB].desc;

		if ((!pUSB_stm32_dev->stm32_INT_IN_EPAddr) &&
		   (pEndpoint_Descriptor->bEndpointAddress & USB_DIR_IN) &&
		   ((pEndpoint_Descriptor->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)== USB_ENDPOINT_XFER_INT)) 
		{
			/* USB IN EP found */
			printk(KERN_INFO "stm32_usb: 3 - EP In found.\n");
			l_MaxPacketSize_UW = pEndpoint_Descriptor->wMaxPacketSize;
			pUSB_stm32_dev->stm32_Rx_Packet_Size = l_MaxPacketSize_UW;
			pUSB_stm32_dev->stm32_INT_IN_EPAddr = pEndpoint_Descriptor->bEndpointAddress;
			pUSB_stm32_dev->stm32_IN_EP_Interval = pEndpoint_Descriptor->bInterval;
			pUSB_stm32_dev->stm32_Rx_Int_Buffer = kmalloc(l_MaxPacketSize_UW, GFP_KERNEL);
			if (pUSB_stm32_dev->stm32_Rx_Int_Buffer == NULL) {
				printk(KERN_INFO "stm32_usb: 3.1 - Rx Int Buffer FAIL allocating.\n");
				goto error;
			}
		}

		if ((!pUSB_stm32_dev->stm32_INT_OUT_EPAddr) &&
		   (pEndpoint_Descriptor->bEndpointAddress & USB_DIR_OUT) &&
		   ((pEndpoint_Descriptor->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)== USB_ENDPOINT_XFER_INT)) 
		{
			/* USB IN EP found */
			printk(KERN_INFO "stm32_usb: 3 - EP Out found.\n");
			l_MaxPacketSize_UW = pEndpoint_Descriptor->wMaxPacketSize;
			pUSB_stm32_dev->stm32_Tx_Packet_Size = l_MaxPacketSize_UW;
			pUSB_stm32_dev->stm32_INT_OUT_EPAddr = pEndpoint_Descriptor->bEndpointAddress;
			pUSB_stm32_dev->stm32_OUT_EP_Interval = pEndpoint_Descriptor->bInterval;
			pUSB_stm32_dev->stm32_Tx_Int_Buffer = kmalloc(l_MaxPacketSize_UW, GFP_KERNEL);
			if (pUSB_stm32_dev->stm32_Tx_Int_Buffer == NULL) {
				printk(KERN_INFO "stm32_usb: 3.1 - Tx Int Buffer FAIL allocating.\n");
				goto error;
			}
		}
	}
	
	/* check if EPs have been found or not ? */
	if ((pUSB_stm32_dev->stm32_INT_OUT_EPAddr == 0) || (pUSB_stm32_dev->stm32_INT_IN_EPAddr == 0))
	{
		printk(KERN_INFO "stm32_usb: 4 - Check EPs FAIL.\n");
		goto error;
	}
	printk(KERN_INFO "stm32_usb: 4 - Check EPs OK.\n");
	
	usb_set_intfdata(interface, pUSB_stm32_dev);

	retval = usb_register_dev(interface, &stm32_class);
	if (retval)
	{
		printk(KERN_INFO "stm32_usb: 5 - register FAIL.\n");
		usb_set_intfdata(interface, NULL);
		goto error;
	}
	else
	{
		printk(KERN_INFO "stm32_usb: 5 - register OK.\n - Minor obtained: %d\n", interface->minor);
	}
	
	return 0;	/* probe successfully. */
	
error:			/* probe fail */
	if (pUSB_stm32_dev != NULL)
	{
		kref_put(&pUSB_stm32_dev->stm32_kref, stm32_delete);
	}
	return retval;
}
/* fixed:end*/

/* fixed:start*/
static void stm32_disconnect(struct usb_interface *interface)
{
	struct usb_stm32 *pUSB_stm32_dev = NULL;
	//lock_kernel();
	pUSB_stm32_dev = usb_get_intfdata(interface);
	usb_set_intfdata(interface, NULL);
	usb_deregister_dev(interface, &stm32_class);
	//unlock_kernel();
	
	kref_put(&pUSB_stm32_dev->stm32_kref, stm32_delete);
	printk(KERN_INFO "usb device disconnected.\n");
}

/* fixed:end*/

/* fixed:start*/
static struct usb_driver stm32_usb_driver = {
	.name = "stm32_int_device",
	.id_table = stm32_usb_id_table,
	.probe = stm32_probe,
	.disconnect = stm32_disconnect,
};
/* fixed:end*/

/* fixed:start*/
static int __init stm32_init(void)
{
	int result;

	/* Register this driver with the USB subsystem */
	result = usb_register(&stm32_usb_driver);
	if (result)
	{
		printk(KERN_ERR "usb_register failed. Error number %d\n", result);
	}
	else
	{
		printk(KERN_INFO "usb_register successfully\n");
	}
	return result;
}
/* fixed:end*/

/* fixed:start*/
static void __exit stm32_exit(void)
{
	usb_deregister(&stm32_usb_driver);
	printk(KERN_INFO "usb_deregister done\n");
}
/* fixed:end*/

/* fixed:start*/
module_init(stm32_init);
module_exit(stm32_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Choen <an23593@gmail.com");
MODULE_DESCRIPTION("stm32f4 discovery usb driver");
/* fixed:end*/
