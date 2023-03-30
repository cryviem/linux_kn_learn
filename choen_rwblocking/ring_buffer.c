#include <linux/module.h>
#include <linux/string.h>
#include <linux/uaccess.h>
#include "ring_buffer.h"

int rb_init(ring_buffer_t* rb)
{
    int i;
    if (rb == NULL)
    {
        return -1;
    }

    for (i = 0; i < NUM_OF_ITEMS; i++)
    {
        memset(rb->item[i].buff, 0, RW_ITEM_SIZE);
        rb->item[i].act_len = 0;
    }
    rb->rptr = 0;
    rb->wptr = 0;
    rb->load_cnt = 0;
    sema_init(&rb->sem, 1);
    return 0;
}

int rb_lock(ring_buffer_t* rb)
{
    return down_interruptible(&rb->sem);
}

void rb_unlock(ring_buffer_t* rb)
{
    up(&rb->sem);
}

int rb_is_writable(ring_buffer_t* rb)
{
    return (rb->load_cnt < NUM_OF_ITEMS);
}

int rb_is_readable(ring_buffer_t* rb)
{
    return (rb->load_cnt > 0);
}

int rb_write(ring_buffer_t* rb, char* pdata, int len)
{
    if (rb->load_cnt >= NUM_OF_ITEMS)
    {
        return -1;
    }

    if (pdata == NULL || len <= 0 || len > RW_ITEM_SIZE)
    {
        return -1;
    }

    memcpy(rb->item[rb->wptr].buff, pdata, len);
    rb->item[rb->wptr].act_len = len;
    rb->wptr = (rb->wptr + 1) % NUM_OF_ITEMS;
    rb->load_cnt++;
    printk(KERN_INFO "rb_write > success write %d bytes, wptr %d, rptr %d, load %d\n", len, rb->rptr, rb->wptr, rb->load_cnt);
    return 0;
}

int rb_read(ring_buffer_t* rb, char* pdata, int len)
{
    int l_len;
    if (rb->load_cnt <= 0)
    {
        return -1;
    }

    if (pdata == NULL || len <= 0)
    {
        return -1;
    }
    l_len = (rb->item[rb->rptr].act_len > len)? len : rb->item[rb->rptr].act_len;
    memcpy(pdata, rb->item[rb->rptr].buff, l_len);
    memset(rb->item[rb->rptr].buff, 0, RW_ITEM_SIZE);
    rb->item[rb->rptr].act_len = 0;
    rb->rptr = (rb->rptr + 1) % NUM_OF_ITEMS;
    rb->load_cnt--;
    printk(KERN_INFO "rb_read > success read %d bytes, wptr %d, rptr %d, load %d\n", l_len, rb->wptr, rb->rptr, rb->load_cnt);
    return l_len;
}

int rb_usr_write(ring_buffer_t* rb, const char __user * pdata, int len)
{
    if (rb->load_cnt >= NUM_OF_ITEMS)
    {
        return -1;
    }

    if (pdata == NULL || len <= 0 || len > RW_ITEM_SIZE)
    {
        return -1;
    }

    if(0 != copy_from_user(rb->item[rb->wptr].buff, pdata, len))
    {
        return -1;
    }
    rb->item[rb->wptr].act_len = len;
    rb->wptr = (rb->wptr + 1) % NUM_OF_ITEMS;
    rb->load_cnt++;
    printk(KERN_INFO "rb_usr_write > success write %d bytes, wptr %d, rptr %d, load %d\n", len, rb->wptr, rb->rptr, rb->load_cnt);
    return 0;
}

int rb_usr_read(ring_buffer_t* rb, char __user * pdata, int len)
{
    int l_len;

    if (rb->load_cnt <= 0)
    {
        return -1;
    }

    if (pdata == NULL || len <= 0)
    {
        return -1;
    }
    l_len = (rb->item[rb->rptr].act_len > len)? len : rb->item[rb->rptr].act_len;
    if(0 != copy_to_user(pdata, rb->item[rb->rptr].buff, l_len))
    {
        return -1;
    }
    memset(rb->item[rb->rptr].buff, 0, RW_ITEM_SIZE);
    rb->item[rb->rptr].act_len = 0;
    rb->rptr = (rb->rptr + 1) % NUM_OF_ITEMS;
    rb->load_cnt--;
    printk(KERN_INFO "rb_usr_read > success read %d bytes, wptr %d, rptr %d, load %d\n", l_len, rb->rptr, rb->wptr, rb->load_cnt);
    return l_len;
}