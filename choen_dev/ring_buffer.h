
#include <asm/semaphore.h>

#define RW_ITEM_SIZE                 256
#define NUM_OF_ITEMS                 4

typedef struct {
    char        buff[RW_ITEM_SIZE];
    int     act_len;
} data_t;

typedef struct {
	data_t      item[NUM_OF_ITEMS];
	int     wptr;
	int     rptr;
	int	    load_cnt;
    struct semaphore sem;
} ring_buffer_t;