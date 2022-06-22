#ifndef  __BLOCK_MANAGER_T__
#define   __BLOCK_MANAGER_T__

#include <stdint.h>

#define CAPACITY_MAX_BLOCKS   8000
#define UPDATE_FINISHED_SEQ   0xFFFF


typedef struct block_manage_type  block_manage_t;


union block_state_type
{
	unsigned char *seq_status;
};
struct block_manage_type
{
    union block_state_type  data;
    uint16_t max_blk_cnt;
	void (*block_reload)(block_manage_t *block,unsigned short max_block_size);
	/*
    ret: 1成功接收包   0:拒绝本包数据
	*/
	char (*block_receive_seq)(block_manage_t *block,unsigned short seq);
	char (*block_is_reveive_all)(block_manage_t *block);
	unsigned char  (*block_get_next_seq)(block_manage_t *block,
	                                       unsigned short end, 
	                                       unsigned short seq[],
	                                       unsigned char max_size);
	void (*block_free)(block_manage_t *block);
};

block_manage_t *get_boardcast_block_manager(void);


#endif 
