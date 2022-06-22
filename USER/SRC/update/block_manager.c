#include "block_manager.h"
#include "config.h"
#include "dataflash.h"
#include "mem_manage.h"
#include "comfunc.h"
#include "stdlib.h"
#include <string.h>

static void  bloardcast_block_init(block_manage_t *block,unsigned short max_block_size)
{
    /*seq: 1 - max_block_size*/
    mem_init();
    block->data.seq_status = mem_alloc(max_block_size+1);
    block->max_blk_cnt = max_block_size;
    memset(block->data.seq_status,0xff,max_block_size+1);
    dataflash_read(PACKAGE_STATUS_ADDRESS,block->data.seq_status,max_block_size+1);
}
static char bloardcast_block_receive_seq(block_manage_t *block,unsigned short seq)
{
    if(seq > block->max_blk_cnt)
    {
        return 0;
    }

    if(block->data.seq_status[seq])
    {
        unsigned char over_data = 0;
        block->data.seq_status[seq] = 0;
        dataflash_write(PACKAGE_STATUS_ADDRESS+seq,&over_data,1);
        return 1;
    }
    return 0;
}
static char  bloardcast_block_is_reveive_all(block_manage_t *block)
{
    /*0: 表示此包已经接受完毕
    */
    return  is_all_xx( &block->data.seq_status[1],0,block->max_blk_cnt);
}
static unsigned char  bloardcast_block_get_next_seq(block_manage_t *block,
        unsigned short end,
        unsigned short seq[],
        unsigned char max_size)
{
    unsigned short i=0;
    int lost_seq =0;
    for(i =1; i<= end; i++)
    {
        if(0xff == block->data.seq_status[i])
        {
            seq[lost_seq] = i;
            lost_seq++;
            if(lost_seq >= max_size)
            {
                break;
            }
        }
    }
    return lost_seq;
}

void  bloardcast_block_free(block_manage_t *block)
{
    mem_free(block->data.seq_status);
}


block_manage_t *get_boardcast_block_manager()
{
    static struct block_manage_type manage;
    manage.block_reload = bloardcast_block_init;
    manage.block_get_next_seq = bloardcast_block_get_next_seq;
    manage.block_is_reveive_all = bloardcast_block_is_reveive_all;
    manage.block_receive_seq = bloardcast_block_receive_seq;
    manage.block_free = bloardcast_block_free;
    return &manage;
}
