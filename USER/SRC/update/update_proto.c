#include "protcl.h"
#include "update.h"
#include "comfunc.h"
#include "alloter.h"
#include "spiflash.h"
#include <stdint.h>
#include "device_info.h"
#include "hardware_layer.h"
#include "update.h"
#include "block_manager.h"
#include "comfunc.h"
#include "uart_fifo.h"

uint16_t sp_crc16_with_init(uint16_t crc, const uint8_t *buf, uint8_t size)
{
    unsigned char i;

    while(size--!=0)
    {
        for(i = 0x80; i != 0; i>>=1)
        {
            if((crc&0x8000) != 0)
            {
                crc <<= 1;
                crc ^= 0x1021;
            }
            else
            {
                crc <<= 1;
            }
            if(((*buf)&i)!=0)
            {
                crc ^= 0x1021;
            }
        }
        buf++;
    }
    return(crc);
}


struct SHS_frame *get_smart_frame(uint8_t rxframe_raw[],uint8_t rxlen,int *clr_len)
{
    struct SHS_frame *pframe;
    uint8_t i=0;
    uint8_t len;
    int frame_data_crc=0;
    int receive_crc;
    int data_len;
    int rubbish_mode=1;

start_lbl:
    while(i < rxlen)
    {
        if(STC == rxframe_raw[i]) break;
        i++;
        if(rubbish_mode)
            *clr_len=i;
    }

    if(rxlen-i < SHS_FRAME_HEAD) return(NULL);
    pframe = (struct SHS_frame*)&rxframe_raw[i];
    len = pframe->length;

    if(len < 7 )
    {
        i++;
        goto start_lbl;
    }

    if(i+SHS_FRAME_HEAD+len+1 > rxlen)
    {
        rubbish_mode = 0;
        i++;
        goto start_lbl;
    }

    if(pframe->infor[len] != checksum((uint8_t *)pframe, len+SHS_FRAME_HEAD))
    {
        i++;
        goto start_lbl;
    }

    frame_data_crc = 0;
    if(CMD_P2P_UPDATE ==  pframe->infor[0])
    {
        data_len = pframe->infor[6];
        frame_data_crc = sp_crc16_with_init(frame_data_crc, &pframe->infor[7], pframe->infor[6]);
    }
    else if(CMD_BOARDCAST_UPDATE  ==  pframe->infor[0])
    {
        data_len =pframe->infor[7];
        frame_data_crc = sp_crc16_with_init(frame_data_crc, &pframe->infor[8], pframe->infor[7]);
    }
    else
    {
        i++;
        goto start_lbl;
    }

    if(0==data_len)
    {
        receive_crc = 0;
    }
    else
    {
        receive_crc = (((pframe->infor[len-1] & 0xff) << 8) + pframe->infor[len-2]);
    }

    if(frame_data_crc !=  receive_crc)
    {
        i++;
        goto start_lbl;
    }

    *clr_len = i+SHS_FRAME_HEAD+len+1;
    pframe = (struct SHS_frame*)&rxframe_raw[i];

    return(pframe);
}

uint8_t repackage_return_frame(struct SHS_frame *pframe, uint8_t len)
{
    uint8_t  aid[ADDRESS_LEN];
    int frame_len=0;
    memcpy(aid,pframe->said,ID_LEN);  //tid and sid exchanged
    memcpy(pframe->said, pframe->taid, ID_LEN);
    memcpy(pframe->taid,aid,ID_LEN);
    pframe->seq |= 0x80;
    pframe->length = len;
    /*确保ack位为0*/
    if(CMD_BOARDCAST_UPDATE == pframe->infor[0])
    {
        pframe->infor[4] = 0;
    }
    else
    {
        pframe->infor[3] =  0;
    }
    pframe->infor[len] = checksum((uint8_t *)pframe, len +SHS_FRAME_HEAD);
    len++;
    /*the total bytes to be sent*/
    frame_len =  (len + SHS_FRAME_HEAD);
    memmove((unsigned char *)(pframe)+6,pframe,frame_len);
    memset(pframe,0xfe,6);
    return  frame_len+6;
}


void _handle_head_package(struct UPDATE_FILE *update_file,
                          unsigned short crc,
                          unsigned short data_len,char mode)
{
    int version_size;
    uint32_t file_size=0;
    version_size = data_len - offset_of(struct UPDATE_FILE, data);
    file_size = *((unsigned int *) update_file->file_sz);
    updater_start(crc,
                  file_size,
                  update_file->blk_sz,
                  (char *)update_file->data,
                  version_size,
                  mode);
    device_set_unpack_magic(PARAMETER_RESET);
}

static void handle_update_frame(struct SHS_frame *pframe)
{
    int frame_len=0;
    unsigned char sub_cmd=0;
    uint16_t seq=0,crc=0,need_reboot=0;
    unsigned char ack=0,data_len=0,head_len=0;
    unsigned char cmd = pframe->infor[0];
    int i=1;// skip cmd
    unsigned char *data;
    struct update_head_type *update_head;

    if(CMD_BOARDCAST_UPDATE == cmd)
    {
        sub_cmd = pframe->infor[i++];
    }
    update_head =(struct update_head_type *)&pframe->infor[i];
    seq = little_bytes_to_uint16_t((&pframe->infor[i]));
    i+=2;
    ack = pframe->infor[i++];
    crc = little_bytes_to_uint16_t((&pframe->infor[i]));
    i+=2;
    data_len =  pframe->infor[i++];
    data = &pframe->infor[i];

    if(CMD_P2P_UPDATE == cmd)
    {
        if(0 == seq)
        {
            sync_switch_high_baud();
            _handle_head_package((struct UPDATE_FILE *)data,crc,data_len,UPDATE_PEER2PEER);
        }
        else
        {
            updater_receive_data(crc, data_len, seq, data);
        }
        seq = updater_get_next_seq();
        uint16_t_to_little_bytes(seq,&pframe->infor[1]);
        data_len = offset_of(struct update_head_type,data)+1; // cmd
        update_head->len = 0;
    }
    else if(CMD_BOARDCAST_UPDATE == cmd)
    {
        unsigned short max_reviced_block;
        head_len = offset_of(struct update_head_type,data)+2;	//cmd sub_cmd
        switch(sub_cmd)
        {
        case BOARDCAST_START:
            _handle_head_package((struct UPDATE_FILE *)data,crc,data_len,UPDATE_BOARDCAST);
            uint16_t_to_little_bytes(CAPACITY_MAX_BLOCKS,data);
            max_reviced_block = updater_get_receive_max_seq();
            uint16_t_to_little_bytes(max_reviced_block,(data+2));
            data_len = 4;
            break;
        case BOARDCAST_DATA:
            updater_receive_data(crc, data_len, seq, data);
            data_len = 0;
            break;
        case BOARDCAST_QUARY_LOST:
            data_len =2* updater_ge_lost_seq(seq,(unsigned short*)data,30);
            break;
        case BOARDCAST_QUARY_RESTART:
            data[0] =  updater_check_file_crc();
            need_reboot =1;
            data_len = 1;
            break;
        }
        update_head->len = data_len;
        data_len += head_len;
    }

    if(ack)
    {
        /*广播升级加长等待时间*/
        if(sub_cmd == CMD_BOARDCAST_UPDATE)
            delay_ms(80);
        frame_len = repackage_return_frame(pframe, data_len);
        sys_uart_sync_write(0,(uint8_t *)pframe,frame_len);
    }

    //最后一帧发送两遍
    if(is_update_finish()&& (CMD_P2P_UPDATE == cmd))
    {
        sync_switch_2400_baud();
        sys_uart_sync_write(0,(uint8_t *)pframe,frame_len);
        hard_reset_now();
    }

    if(is_update_finish()&&need_reboot)
    {
        sys_uart_sync_write(0,(uint8_t *)pframe,frame_len);
        hard_reset_now();
    }
}


int handle_update_task(void)
{
    uint8_t buffer[0x100];
    uint8_t devid_ID[7];
    int peek_len;
    int clr_len=0;
    struct SHS_frame *pframe;

    watchdog_set_free_feed_time(10);
    peek_len = sys_uart_peek_data(0,buffer,sizeof(buffer));
    pframe = get_smart_frame(buffer,peek_len,&clr_len);//if 7E frame complete

    if(NULL == pframe)
    {
        if(is_in_updateing() && 0 != clr_len)
        {
            sys_uart_read(0,buffer,clr_len);
        }

        return (0);
    }

    sys_uart_read(0,buffer,peek_len);
    device_get_devid(devid_ID);

    memmove(buffer,pframe,(unsigned char *)pframe- buffer);
    pframe = (struct SHS_frame *)pframe;

    updater_receive_data_hook();

    if(0 == memcmp(&devid_ID[0],pframe->said,PROTO_188_ADDRESS_LEN)
            || is_all_xx(pframe->said,0xaa,7))
    {
        int time;
        srand(devid_ID[6]);
        time = rand()%100; // 升级一帧的时间是900ms
        delay_ms(time);
        handle_update_frame(pframe);
    }

    watchdog_clr_free_feed_time();
    return (0);
}
