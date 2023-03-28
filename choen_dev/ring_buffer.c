
#include <linux/string.h>
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

