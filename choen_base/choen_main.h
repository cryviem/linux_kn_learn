#include <linux/ioctl.h>

#define CHOEN_MAGIC_NUMBER  'x'
#define IOCTL_CMD_HELLO             _IO(CHOEN_MAGIC_NUMBER, 1)
#define IOCTL_CMD_SET_VALUE         _IO(CHOEN_MAGIC_NUMBER, 2)
#define IOCTL_CMD_SET_PTR           _IOW(CHOEN_MAGIC_NUMBER, 3, int)
#define IOCTL_CMD_GET_PTR           _IOR(CHOEN_MAGIC_NUMBER, 4, int)

#define DYNAMIC_DEV_NODE    
