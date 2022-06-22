#include "update.h"
#include "hardware_layer.h"
#include "device_info.h"
#include "mem_manage.h"
#include "block_manager.h"
#include "config.h"
#include <string.h>
#include "comfunc.h"
#include "protcl.h"

enum
{
    UPDATE_WAITING=0,
    UPDATE_UPDATEING,
    UPDATE_COMPLETE
};

static update_state_t  update_tmp= {0,0,0,0,0,0,UPDATE_WAITING,{0},0};

static void _write_update_falg(void)
{
    unsigned char tmp = UPDATE_MAGIC_VALUE;
    dataflash_erase(PROGRAM_END_ADDRESS, 1);
    dataflash_write(PROGRAM_END_ADDRESS,&tmp,1);
}

void updater_start(unsigned short crc,
                   unsigned int file_sz,
                   unsigned char block_sz,
                   char version[],
                   char ver_len,
                   char mode)
{
    update_tmp.mode = mode;
    update_tmp.timeouts = UPDATE_MAX_TIMEOUT;
    /*断点续传,广播升级和点对点升级可以混用*/
    if((update_tmp.file_sz == file_sz)
            &&(update_tmp.file_crc == crc)
            &&(update_tmp.block_sz == block_sz)
            &&(0x00 == memcmp(update_tmp.version,version, ver_len)))
    {
        if(UPDATE_WAITING == update_tmp.state)
        {
            update_tmp.block_manager->block_reload(update_tmp.block_manager,
                                                   update_tmp.max_block_cnt);

        }
        update_tmp.state = UPDATE_UPDATEING;
        return;
    }

    update_tmp.state = UPDATE_UPDATEING;
    /*重新升级，清空Flash数据和标志位*/
    dataflash_erase(SPI_PROGRAM_ADDRESS,BLOCK_SIZE*16);
    memcpy(update_tmp.version, version, ver_len);
    update_tmp.file_sz = file_sz;
    update_tmp.block_sz = block_sz;
    update_tmp.start_reviced_block = 1;
    update_tmp.file_crc = crc;
    update_tmp.block_manager = get_boardcast_block_manager();
    update_tmp.max_block_cnt= ((update_tmp.file_sz + (uint32_t)update_tmp.block_sz - 1)/(uint32_t)update_tmp.block_sz);
    update_tmp.block_manager->block_reload(update_tmp.block_manager,update_tmp.max_block_cnt);
}

unsigned char updater_receive_data(unsigned short crc,
                                   unsigned char length,
                                   unsigned short seq,
                                   unsigned char *data)
{
    block_manage_t *manage = update_tmp.block_manager;
    int address;

    update_tmp.timeouts = UPDATE_MAX_TIMEOUT;

    if((crc == update_tmp.file_crc)
            && (length == update_tmp.block_sz)
            && (UPDATE_UPDATEING == update_tmp.state))
    {
        if(update_tmp.start_reviced_block < seq)
        {
            update_tmp.start_reviced_block = seq;
        }

        if(manage->block_receive_seq(manage,seq))
        {
            address = (seq-1) * update_tmp.block_sz+SPI_PROGRAM_ADDRESS;
            dataflash_write(address, data, length);
        }

        if(manage->block_is_reveive_all(manage) &&
                UPDATE_PEER2PEER == update_tmp.mode)
        {
            updater_check_file_crc();
        }
        return 1;
    }
    else
    {
        return 0;
    }


}

unsigned short updater_get_next_seq()
{
    if( (UPDATE_WAITING !=update_tmp.state)
            && (UPDATE_PEER2PEER == update_tmp.mode))
    {
        block_manage_t *manage = update_tmp.block_manager;
        unsigned short seq=0;
        unsigned short count;
        count = manage->block_get_next_seq(manage,update_tmp.max_block_cnt,&seq,1);
        if(0 == count)
            seq = UPDATE_FINISHED_SEQ;
        return seq;
    }
    return 0;
}


unsigned short updater_get_receive_max_seq()
{
    return  update_tmp.start_reviced_block;

}


unsigned short updater_ge_lost_seq(unsigned short end,
                                   unsigned short seq[],
                                   unsigned short max_count)
{
    if( (UPDATE_WAITING !=update_tmp.state)
            && (UPDATE_BOARDCAST == update_tmp.mode))
    {
        block_manage_t *manage = update_tmp.block_manager;
        return manage->block_get_next_seq(manage,end,seq,max_count);
    }

    memset(seq,0xff,max_count*2);
    return max_count;
}

uint16_t dataflash_calc_crc16(int start,int len)
{
    unsigned char buff[128]= {0};
    int read_length=0;
    unsigned short crc=0;
    while (len > 0)
    {
        read_length = min(len,sizeof(buff));
        dataflash_read(start,buff,read_length);
        crc = sp_crc16_with_init(crc, buff, read_length);
        start+=read_length;
        len -= read_length;
    }
    return crc;
}

unsigned char updater_check_file_crc()
{
    int crc=0, flash_addr;
    unsigned short seq=0;
    block_manage_t *manage = update_tmp.block_manager;

    if((UPDATE_WAITING == update_tmp.state)
            ||  (0 == update_tmp.file_sz))
    {
        return CRC_NO_UPDATE;
    }

    if(0 != manage->block_get_next_seq(manage,update_tmp.max_block_cnt,&seq,1))
    {
        return CRC_NEED_PACKAGE;
    }

    flash_addr = SPI_PROGRAM_ADDRESS;
    hardware_init_high_power();
    delay_ms(200); //等待电压稳定
    crc = dataflash_calc_crc16(flash_addr,update_tmp.file_sz);
    hardware_init_low_power();

    if(UPDATE_PEER2PEER == update_tmp.mode)
    {
        if(devinfo.highspeed_baud > 9600)
        {
            hardware_init_high_power();
        }
        usart_init(devinfo.highspeed_baud,1);
    }

    if(crc == update_tmp.file_crc)
    {
        _write_update_falg();
        update_tmp.state = UPDATE_COMPLETE;
        return 1;
    }
    else
    {
        return  0;
    }
}

int is_update_finish(void)
{
    return  update_tmp.state == UPDATE_COMPLETE;
}

int is_in_updateing(void)
{
    return  update_tmp.state == UPDATE_UPDATEING;
}

void updater_receive_data_hook()
{
    update_tmp.timeouts = UPDATE_MAX_TIMEOUT;
}

void update_1s_tick()
{
    block_manage_t *manage = update_tmp.block_manager;

    if(UPDATE_UPDATEING != update_tmp.state)
    {
        return;
    }
    if(update_tmp.timeouts > 0)
    {
        update_tmp.timeouts--;
        return;
    }

    if(is_update_finish())
    {
        hard_reset_now();
    }


    sync_switch_2400_baud();
    manage->block_free(manage);
    update_tmp.state = UPDATE_WAITING;
}


