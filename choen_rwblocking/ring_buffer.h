
#include <linux/semaphore.h>

#define RW_ITEM_SIZE                 256
#define NUM_OF_ITEMS                 4

typedef struct {
    char        buff[RW_ITEM_SIZE];
    int         act_len;
} data_t;

typedef struct {
	data_t              item[NUM_OF_ITEMS];
	int                 wptr;
	int                 rptr;
	int	                load_cnt;
    struct semaphore    sem;
} ring_buffer_t;

int rb_init(ring_buffer_t* rb);
int rb_lock(ring_buffer_t* rb);
void rb_unlock(ring_buffer_t* rb);
int rb_is_writable(ring_buffer_t* rb);
int rb_is_readable(ring_buffer_t* rb);
int rb_write(ring_buffer_t* rb, char* pdata, int len);
int rb_read(ring_buffer_t* rb, char* pdata, int len);
int rb_usr_write(ring_buffer_t* rb, const char __user * pdata, int len);
int rb_usr_read(ring_buffer_t* rb, char __user * pdata, int len);