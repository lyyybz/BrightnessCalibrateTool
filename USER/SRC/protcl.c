#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>
#include <assert.h>
#include "config.h"
#include "protcl.h"
#include "comfunc.h"
#include "device_info.h"
#include "uart_fifo.h"
#include "main.h"
#include "camera.h"
#include "recognize.h"
#include "hardware_layer.h"
#include "usart.h"
#include "device_info.h"
#include "update.h"
#include "bmp_layer.h"
#include "update.h"
#include "hash.h"
#include "comfunc.h"


static unsigned char global_ser = 0;
static unsigned short frame_did;
int peek_len;
int is_my_frame;

static int is_need_delay =1;

int handle_proto_188_task(void)
{
    uint8_t buffer[0x100];
    int idx,framelen;
    int call_me= 0;
    int boardcast = 0;
    int task;
    /*从串口缓冲区中读数据*/
    peek_len = sys_uart_peek_data(0,buffer,sizeof(buffer));
    /*解析地址段*/
    is_my_frame = get_my_frame(buffer,peek_len);
    if(is_my_frame)
    {
        //ldo_enable(0);
    }
    /*寻找完整帧*/
    idx = get_188_frame(buffer,peek_len,&call_me,&boardcast);
    if (idx >= 0)
    {
        if ((call_me) || (boardcast))//call me or boardcast
        {
            framelen =  buffer[idx+10] +11 + 2;
            sys_uart_read(0,buffer,idx);
            sys_uart_read(0,buffer,framelen);
            /*处理完整帧*/
            task = handle_write_read_cmd(buffer);
            if (task > 0)
            {
                return (task);//执行task时，在do_task中ldo_enable(1)
            }
            else
            {
                //ldo_enable(1);
            }
        }
        else  //188frame not for me
        {
            return (0);
        }
    }

    return (0);
}

void send_eastsfot_error_code(int cmd,char code)
{
    unsigned char frame[0x20];
    int idx = 0;
    frame[idx++] = 0xFE;
    frame[idx++] = 0xFE;
    frame[idx++] = 0x68;
    frame[idx++] = 0x10;
    device_get_devid(&frame[idx]);
    idx+= 7;
    frame[idx++] = 0xC0|cmd;
    frame[idx++] = 5;
    frame[idx++] = frame_did & 0xff;
    frame[idx++] = frame_did >> 8;
    frame[idx++] = global_ser;
    frame[idx++] = 0;
    frame[idx++] = code;
    frame[idx] = checksum(frame+2,idx-2);
    idx++;
    frame[idx++] = 0x16;
    sys_uart_sync_write(0,frame,idx);
}

#define COLD_WATER_METER   0x10
int send_raw_188_frame(unsigned char cmd,int did,int length, unsigned char data[])
{
    uint8_t buff[0x80];
    uint8_t address[7];
    int idx = 0;

    if(is_need_delay)
    {
        delay_ms(100);
    }
    device_get_devid(address);
	
	
	//delay 2 Byte
	if(2400 == device_get_low_baud())
	{
		delay_ms(4);
	}
	else if(1200 == device_get_low_baud())
	{
		delay_ms(9);
	}
    buff[idx++] = 0xFE;
    buff[idx++] = 0xFE;
    buff[idx++] = 0xFE;
    buff[idx++] = 0xFE;
    buff[idx++] = 0x68;
    buff[idx++] = COLD_WATER_METER;
    memcpy(buff+idx,devinfo.devid,7);
    idx+= 7;
    buff[idx++] =  cmd;//0xC0 | cmd
    buff[idx++] = 3+length;
    buff[idx++] = did&0xff;
    buff[idx++] = did>>8;
    buff[idx++] = global_ser;
    memcpy(buff+idx,data,length);
    idx += length;
    buff[idx++] = checksum(buff+4,3 + 11 + length);
    buff[idx++] = 0x16;
    sys_uart_sync_write(0,buff,idx);
    return idx;
}

int send_response_188_frame(unsigned char cmd,int did,int length, unsigned char data[])
{
    return send_raw_188_frame(cmd|0x80,did,length,data);
}

int get_188_frame(unsigned char rxframe_raw[],int rxlen,int *call_my_addr,int *boardcast_my)
{
    int  i = 0,datalen;
    uint8_t cs;
    int framelen;
    unsigned char devid[7];

    if (rxlen < 9) return(-1);
check_frame:

    *call_my_addr = 0;
    *boardcast_my = 0;
    while(i < rxlen)
    {
        if(0x68 == rxframe_raw[i])//188 head
            break;
        i++;
    }
    if(i>=rxlen) return(-1);//if not 118 head return -1
    framelen = rxlen - i;// frame len

    if((!(is_data_all_xx(&rxframe_raw[i + 1 ],1, 0x10))) && (!(is_data_all_xx(&rxframe_raw[i + 1 ],1, 0xAA))))//if not water or boardcast
    {
        return (-1);
    }

    if (framelen >= 9)
    {
        device_get_devid(devid);
        if (0 == memcmp(&rxframe_raw[i + 2 ],devid, 7))//address equal
        {
            if(call_my_addr)  *call_my_addr = 1;
        }
        if(is_all_xx(&rxframe_raw[i + 2 ],0xAA, 7))//boardcast
        {
            if(boardcast_my) *boardcast_my = 1;
        }
    }
    if (framelen < 11)  return (-1);
    datalen = rxframe_raw[i+10];//datalen
    if (datalen + 11 + 2 > framelen)
    {
        i++;
        goto check_frame;
    }
    cs = checksum(rxframe_raw+i, datalen  + 11 );
    if(rxframe_raw[i + datalen  + 11 ] != cs)
    {
        i++;
        goto check_frame;
    }
    if(rxframe_raw[i + datalen  + 11 + 1] != 0x16)//not equal end symbol
    {
        i++;
        goto check_frame;
    }
    return(i);
}

//解析报文地址段:
//不匹配本机地址返回 0
//匹配本机地址返回   1
int get_my_frame(unsigned char rxframe_raw[],int rxlen)
{
    int i = 0,frame_len;
    unsigned char devid[7];

    if (rxlen < 9) return(0);

    while(i < rxlen)
    {
        if(0x68 == rxframe_raw[i])//find 188 head
            break;
        i++;
    }
    if(i>=rxlen) return (0);//if not 118 head return -1

    if(rxframe_raw[i+1]!= 10)//not water meter
        return (0);
    frame_len = rxlen - i;// frame len

    if(frame_len >= 9)
    {
        device_get_devid(devid);
        if (0 == memcmp(&rxframe_raw[i + 2 ],devid, 7))//address equal
        {
            return (1);
        }
        else
        {
            return (0);
        }
    }
    else
    {
        return (0);
    }

}

#define get_short_para(buff,i)   ((buff[i*2+1]<<8) + buff[i*2])
void set_short_para(unsigned char buff[],int index,short value)
{
    buff[index*2] = (value)&0xff;
    buff[index*2+1]=(value)>>8;
}

short get_short_para_(unsigned char buff[],int i)
{
    return ((buff[i*2+1]<<8) + buff[i*2]);
}

#ifdef ESSN_WCR_P
static int pointer_window_read(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    unsigned char buff[35],i;
    unsigned char pointer_count = devinfo.digit_count;

    memset(buff,0x00,sizeof(buff));
    set_short_para(buff,0,devinfo.pi.pointer_width);
    for(i=0; i<pointer_count; i++)
    {
        set_short_para(buff,i*2+1,devinfo.pi.center[i].x);
        set_short_para(buff,i*2+2,devinfo.pi.center[i].y);
    }

    send_response_188_frame(proto->cmd,did,MAX_POINTER_COUNT*4+2,buff);//MAX_POINTER_COUNT*4+2个字节数据长度
    return 0;
}

static int pointer_window_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    volatile  unsigned char i=0;
    unsigned char pointer_count = devinfo.digit_count;//字轮数 = 指针数
    point_t  center[MAX_POINTER_COUNT];
    short    pointer_width;

    if(w_len < pointer_count*4 +2)
    {
        return ERROR_MESSAGE_FORMAT;
    }

    pointer_width = get_short_para(buff,0);
    for(i=0; i < pointer_count; i++)
    {
        center[i].x = get_short_para_(buff,i*2+1);
        center[i].y = get_short_para_(buff,i*2+2);
    }

    if(device_save_pointers_info(center,pointer_width))
    {
        return -1;
    }

    send_response_188_frame(proto->cmd,did,0,0);

    return 0;
}
#else
static int digit_window_read(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    unsigned char buff[28]= {0};
    quadrange_t  *qr = &devinfo.di.digital_quadrange;

#ifdef ESSN_WCR_A
    set_short_para(buff,0,qr->p00.x);
    set_short_para(buff,1,qr->p00.y);
    set_short_para(buff,2,qr->p01.x);
    set_short_para(buff,3,qr->p01.y);
    set_short_para(buff,4,qr->p10.x);
    set_short_para(buff,5,qr->p10.y);
    set_short_para(buff,6,qr->p11.x);
    set_short_para(buff,7,qr->p11.y);
#else
    set_short_para(buff,0,CMAERA_MAX_WIDTH - qr->p01.x);
    set_short_para(buff,1,qr->p00.y);
    set_short_para(buff,2,CMAERA_MAX_WIDTH - qr->p00.x);
    set_short_para(buff,3,qr->p01.y);
    set_short_para(buff,4,CMAERA_MAX_WIDTH - qr->p11.x);
    set_short_para(buff,5,qr->p10.y);
    set_short_para(buff,6,CMAERA_MAX_WIDTH - qr->p10.x);
    set_short_para(buff,7,qr->p11.y);

#endif

    set_short_para(buff,8,devinfo.di.digit_offset[0]);
    set_short_para(buff,9,devinfo.di.digit_offset[1]);
    set_short_para(buff,10,devinfo.di.digit_offset[2]);
    set_short_para(buff,11,devinfo.di.digit_offset[3]);
    set_short_para(buff,12,devinfo.di.digit_offset[4]);
    set_short_para(buff,13,devinfo.di.digit_offset[5]);

    send_response_188_frame(proto->cmd,did,sizeof(buff),buff);
    return 0;
}

static int digit_window_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    quadrange_t  quad;
    short offset[MAX_DIGIT_CNT];

#ifdef ESSN_WCR_A
    quad.p00.x = get_short_para(buff,0);
    quad.p00.y = get_short_para(buff,1);
    quad.p01.x = get_short_para(buff,2);
    quad.p01.y = get_short_para(buff,3);
    quad.p10.x = get_short_para(buff,4);
    quad.p10.y = get_short_para(buff,5);
    quad.p11.x = get_short_para(buff,6);
    quad.p11.y = get_short_para(buff,7);

#else
    quad.p00.x = CMAERA_MAX_WIDTH - get_short_para(buff,2);
    quad.p00.y = get_short_para(buff,1);
    quad.p01.x = CMAERA_MAX_WIDTH - get_short_para(buff,0);
    quad.p01.y = get_short_para(buff,3);
    quad.p10.x = CMAERA_MAX_WIDTH - get_short_para(buff,6);
    quad.p10.y = get_short_para(buff,5);
    quad.p11.x = CMAERA_MAX_WIDTH - get_short_para(buff,4);
    quad.p11.y = get_short_para(buff,7);
#endif

    offset[0] = get_short_para(buff,8);
    offset[1] = get_short_para(buff,9);
    offset[2] = get_short_para(buff,10);
    offset[3] = get_short_para(buff,11);
    offset[4] = get_short_para(buff,12);
    offset[5] = get_short_para(buff,13);

    if(device_save_digits_info(&quad,offset))
    {
        return -1;
    }

    send_response_188_frame(proto->cmd,did,0,0);

    return 0;
}

#endif

static int factory_reset_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    device_info_factory_reset();
    return 0;
}
static int set_out_factory(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    device_enter_factory_mode();
    send_response_188_frame(proto->cmd,did,0,0);
    return 0;
}

static int set_flash_init_default(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    device_init_default();
    send_response_188_frame(proto->cmd,did,0,0);
    return 0;
}

#if CONFIG_DEBUG
static int motor_control_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 1)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    motor_direction = buff[0];
    send_response_188_frame(proto->cmd,did,0,0);
    return TASK_MOTOR;
}

static int crop_window_read(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    unsigned char buff[8];
    set_short_para(buff,0,devinfo.crop_rect.left);
    set_short_para(buff,1,devinfo.crop_rect.right-devinfo.crop_rect.left);
    set_short_para(buff,2,devinfo.crop_rect.top);
    set_short_para(buff,3,devinfo.crop_rect.bottom - devinfo.crop_rect.top);

    send_response_188_frame(proto->cmd,did,sizeof(buff),buff);
    return 0;
}

static int crop_window_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    struct TRECT  rect;

    rect.left = get_short_para(buff, 0);
    rect.right =  rect.left + get_short_para(buff, 1);
    rect.top = get_short_para(buff,2);
    rect.bottom  = rect.top  + get_short_para(buff,3);

    if(device_save_crop_rect(&rect))
    {
        return ERROR_MESSAGE_FORMAT;
    }

    send_response_188_frame(proto->cmd,did,0,0);


    comos_0308_write_windows_rect(devinfo.crop_rect.left,
                                  devinfo.crop_rect.top,
                                  device_get_windows_height(),
                                  device_get_windows_width());
    return 0;
}
#endif

static int address_write(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < PROTO_188_ADDRESS_LEN)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    if(is_meter_in_factory_mode())
    {
        return ERROR_SETED_FACTORY_MODE;
    }
    device_set_devid(&buff[0]);
    force_update_devinfo();
    send_response_188_frame(proto->cmd,did,0,0);
    return 0;
}

static int address_read(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    send_response_188_frame(proto->cmd,did,sizeof(devinfo.devid),devinfo.devid);
    return 0;
}

static int get_digit_count(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    send_response_188_frame(proto->cmd,did,sizeof(devinfo.digit_count),&devinfo.digit_count);
    return 0;
}

static int set_water_digit_count(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 1)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    if(device_save_digit_count(buff[0]) == 0)
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}

static int get_digit_width(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    unsigned char  width = device_get_digit_width();
    send_response_188_frame(proto->cmd,did,sizeof(width),&width);
    return 0;
}

static int set_digit_width(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 1)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    if(device_save_digit_width(buff[0]) == 0)
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}

static int get_color_reversal(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    send_response_188_frame(proto->cmd,did,sizeof(devinfo.color_reverse),&devinfo.color_reverse);
    return 0;
}
static int set_color_reversal(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 1)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    if(device_save_color_reversal(buff[0]) == 0)
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}

static int set_highspeed_baud(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 2)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    int baud = get_short_para(buff,0);
    if(device_save_high_baud(baud))
    {
        return ERROR_MESSAGE_FORMAT;
    }
    send_response_188_frame(proto->cmd,did,0,0);
    return 0;
}

static int read_highspeed_baud(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    unsigned char buff[2];
    set_short_para(buff,0,devinfo.highspeed_baud);
    send_response_188_frame(proto->cmd,did,sizeof(buff),buff);
    return 0;
}


static int set_light_pwm(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len < 1)
    {
        return ERROR_MESSAGE_FORMAT;
    }
    if(device_save_light_pwm(buff[0]) == 0)
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}

static int read_light_pwm(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    send_response_188_frame(proto->cmd,did,1,&devinfo.light_pwm);
    return 0;
}

static int read_info_summary(proto_188_t  *proto, uint8_t buff1[], uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    uint8_t arg_version[12]= {0};
    uint8_t buffer[80]= {0};
    unsigned char idx = 0;

    memcpy(&buffer,PROGRAM_VERSION_1,strlen(PROGRAM_VERSION_1));
    idx += strlen(PROGRAM_VERSION_1);

    memcpy(&buffer[idx],VERSION_PREFIX,strlen(VERSION_PREFIX));
    idx += strlen(VERSION_PREFIX);

    dataflash_read(CNN_VERSION_ADDR,&arg_version,sizeof(arg_version));
    memcpy(&buffer[idx],&arg_version,sizeof(arg_version));
    idx += sizeof(arg_version);

    buffer[idx] = is_device_from_eastsoft();
    idx++;

    send_response_188_frame(proto->cmd,did,idx,buffer);

    return 0;
}

static int read_mcu_id(proto_188_t  *proto, uint8_t  buff1[],uint8_t arg_len, uint8_t max_len, uint16_t did)
{
    uint32_t unique_ID[3];
    uint8_t buffer[12];
    int i = 0;
    Get_ChipID(unique_ID);
    for(i = 0; i < 3; i++)
    {
        put_be_val(unique_ID[i], &buffer[i*4], 4);
    }
    send_response_188_frame(proto->cmd,did,sizeof(buffer),buffer);
    return 0;
}

static int set_mcu_hash(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    memcpy(&devinfo.mcu_id_hash,buff,4);
    force_update_devinfo();
    send_response_188_frame(proto->cmd,did,0,0);
    return 0;
}

static int set_num_factor(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if((w_len == 1) && (0==device_save_num_factor(buff[0])))
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return  0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}


static int get_num_factor(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    send_response_188_frame(proto->cmd,did,1,(unsigned char *)&devinfo.num_factor);
    return 0;
}


static int set_use_previous_result(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    if(w_len == 1)
    {
        device_set_use_previous_flag(buff[0]);
        send_response_188_frame(proto->cmd,did,0,0);
        return  0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}


static int get_use_previous_result(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    unsigned char flag = device_is_use_previous_result();
    send_response_188_frame(proto->cmd,did,1,&flag);
    return 0;
}


static int set_number_mask(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{

    if(w_len <= 1)
    {
        device_reset_check_mask();
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }

    if(0 == device_set_check_mask(buff+1,w_len-1,buff[0]))
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return  0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}


static int get_number_mask(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    unsigned char buff[30];
    int count = device_get_check_mask(buff,sizeof(buff));
    send_response_188_frame(proto->cmd,did,count,buff);
    return 0;
}


static int set_dirty_digit_mask(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{

    if(w_len <= 1)
    {
        device_reset_dirty_digit_mask();
        send_response_188_frame(proto->cmd,did,0,0);
        return 0;
    }

    if(0 == device_set_dirty_digit_mask(buff+1,w_len-1,buff[0]))
    {
        send_response_188_frame(proto->cmd,did,0,0);
        return  0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}


static int get_dirty_digit_mask(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    unsigned char buff[30];
    int count = device_get_dirty_digit_mask(buff,sizeof(buff));
    send_response_188_frame(proto->cmd,did,count,buff);
    return 0;
}


/*表地址共有14位地址*/
static int search_meter(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    int i=0;
    int match = 1;
    uint8_t *match_pattern = buff1 + 1;

    if(arg_len < 15)
    {
        return ERROR_MESSAGE_FORMAT;
    }

    if(device_is_id_slient(buff1[0]))
    {
        return 0;
    }

    for(i=0; i<14; i++)
    {
        int top,bottom;
        bottom = (match_pattern[i]&0xf0)>>4;
        top = (match_pattern[i]&0x0f);
        if(0 == device_is_address_in_range(i, bottom,top))
        {
            match = 0;
        }
    }

    if(match)
    {
        send_response_188_frame(proto->cmd,did,sizeof(devinfo.devid),devinfo.devid);
    }

    return 0;
}

static int slient_search_meter(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    unsigned char id = 0;
    int count = 0;
    int i = 0;
    unsigned char devid[7];

    if(w_len < 8)
    {
        return ERROR_MESSAGE_FORMAT;
    }

    device_get_devid(devid);
    id = buff[0];
    count = (w_len-1)/7;
    for(i=0; i<count; i++)
    {
        if(0 == memcmp(devid,buff+1+7*i,sizeof(devid)))
        {
            device_set_slient_id(id);
        }
    }

    return 0;
}


static int get_low_baud(proto_188_t  *proto,uint8_t  buff1[],uint8_t arg_len, uint8_t max_len,uint16_t did)
{
    unsigned short baud = device_get_low_baud();
    send_response_188_frame(proto->cmd,did,2,(unsigned char *) &baud);
    return 0;
}


static int set_low_baud(proto_188_t  *proto,uint8_t buff[],uint8_t w_len, uint16_t did)
{
    int baud = get_short_para(buff,0);

    if(w_len < 2)
    {
        return ERROR_MESSAGE_FORMAT;
    }

    if((w_len ==2) && (0 == device_save_low_baud(baud)))
    {
        send_response_188_frame(proto->cmd,did,0,0);
        usart_init(device_get_low_baud(),  1);
        return  0;
    }
    else
    {
        return ERROR_MESSAGE_FORMAT;
    }
}


const proto_188_item_t  proto_188_item_list[]=
{
#ifdef ESSN_WCR_P
    {DID_POINTER_WINDOWS,   pointer_window_read,   pointer_window_write,  -1},
#else
    {DID_DIGIT_WINDOWS,     digit_window_read,     digit_window_write,    -1},
#endif
    {DID_FACTORY_RESET,     NULL,                  factory_reset_write,   -1},
    {DID_WATER_USAGE,       NULL,                  NULL,                  TASK_READ_METER},
    {DID_RAW_IMAGE_JPEG,    NULL,                  NULL,                  TASK_GET_RAW_IMG_JPEG},
    {DID_DIGIT_IMG,         NULL,                  NULL,                  TASK_GET_DIGIT_IMG_JPEG},
    {DID_WRITE_ADDRESS,     NULL,                  address_write,         -1},
    {DID_READ_ADDRESS,      address_read,          NULL,                  -1},
    {DID_CNN_TEST,		    NULL,			      NULL,				     TASK_TEST_CNN},
    {DID_HIGHSPEED_BAUD,    read_highspeed_baud,   set_highspeed_baud,    -1},
    {DID_LIGHT_PWM,		    read_light_pwm,	      set_light_pwm,	     -1},
    {DID_RAW_NUMBER,        NULL,                  NULL,                  TASK_GET_RAW_NUMBER},
    {DID_INFO_SUMMARY,	    read_info_summary,	  NULL,				     -1},
    {DID_SET_OUT_FACTORY,   NULL,	              set_out_factory,		 -1},
    {DID_SET_FLAHS_INIT_DEFAUTL,  NULL,	          set_flash_init_default,		 -1},
    {DID_READ_MCU_ID,       read_mcu_id,	          NULL,		             -1},
    {DID_SET_MUC_HASH,      NULL,                  set_mcu_hash,          -1},
    {DID_NUM_FACTOR,        get_num_factor,        set_num_factor,        -1},
    {DID_USE_PREVIOUS_RESULT,get_use_previous_result,set_use_previous_result, -1},
    {DID_NUMBER_CHECK_MASK, get_number_mask, set_number_mask, -1},
    {DID_TRAIN_SAMPLE,	   NULL,	  NULL, 	 TASK_GET_TRAIN_DIGIT_IMG},
    {DID_DIGIT_COUNT,  get_digit_count,  set_water_digit_count,  -1},
    {DID_DIGIT_WIDTH,  get_digit_width,  set_digit_width,  -1},
    {DID_COLOR_REVERSAL,  get_color_reversal,  set_color_reversal,  -1},
    {DID_DIRTY_DIGIT_MASK,  get_dirty_digit_mask,  set_dirty_digit_mask,  -1},
    {DID_SEARCH_METER,  search_meter,  NULL,  -1},
    {DID_SILENT_METER,  NULL,  slient_search_meter,  -1},
    {DID_LOW_BAUD,  get_low_baud,  set_low_baud,  -1},

#if CONFIG_DEBUG
    {DID_RAW_IMAGE_BMP, 	NULL,					NULL,		   TASK_GET_RAW_IMG_BMP},
    {DID_MODE_CMD,			NULL,				   NULL,				  TASK_CMD_MODE},
    {DID_CROP_WINDOWS,		crop_window_read,	   crop_window_write,	  -1},
    {DID_MOTOR_CONTROL, 	NULL,				   motor_control_write,   -1},
#endif
};

static void _send_baud_swtich_frame(int baud)
{
    unsigned char buff[2]= {0};
    set_short_para(buff,0,(unsigned short)baud);
    send_raw_188_frame(CMD_188_WRITE,DID_SWITCH_BAUD, sizeof(buff),buff);
    send_raw_188_frame(CMD_188_WRITE,DID_SWITCH_BAUD, sizeof(buff),buff);
    delay_ms(300);
}

void sync_switch_high_baud()
{
    _send_baud_swtich_frame(devinfo.highspeed_baud );

    if(devinfo.highspeed_baud > 9600)
    {
        hardware_init_high_power();
    }
    usart_init(devinfo.highspeed_baud,1);
}

void sync_switch_2400_baud()
{
    _send_baud_swtich_frame(device_get_low_baud());

    if(devinfo.highspeed_baud > 9600)
    {
        hardware_init_low_power();
    }

    usart_init(device_get_low_baud(),1);
}

int handle_write_read_cmd(unsigned char data[])
{
    int task =0;
    int i=0;
    is_need_delay =1;

    const proto_188_item_t *item = NULL;
    proto_188_t  *proto = (proto_188_t *)data;
    unsigned short reverse_did = ((proto->did[0] & 0xff) << 8) + proto->did[1];
    unsigned short normal_did = ((proto->did[1] & 0xff) << 8) + proto->did[0];
    frame_did = normal_did;
    global_ser = proto->data[0];
    for (i=0; i < ARRAY_SIZE(proto_188_item_list); i++)
    {
        item = &proto_188_item_list[i];
        if(item->did == reverse_did || item->did == normal_did)
        {
            switch(proto->cmd)
            {
            case CMD_188_READ:
                if(NULL != item->read)
                {
                    task = item->read(proto,proto->data+1,proto->len-3,30,normal_did);
                }
                else
                {
                    is_need_delay = 0;
                    task = item->dafault_task;
                }
                break;
            case CMD_188_READ_ADDRESS:
                if(NULL != item->read)
                {
                    task = item->read(proto,proto->data+1,proto->len-3,30,normal_did);
                }
                else
                {
                    task = item->dafault_task;
                }
                break;
            case CMD_188_SET_ADDRESS:
                if(NULL != item->write)
                {
                    task = item->write(proto,proto->data+1,proto->len-3,normal_did);
                }
                break;
            case CMD_188_WRITE:
                if(NULL != item->write)
                {
                    task = item->write(proto,proto->data+1,proto->len-3,normal_did);
                }
                else
                {
                    task = item->dafault_task;
                }
                break;
            default :
                break;
            }
        }
    }
    if(task < 0)
    {
        send_eastsfot_error_code(proto->cmd,(char)task);
    }

    return task;
}


void send_meter_usage_number(char        digit[6], unsigned char digit_cnt)
{
    unsigned char buff[19] = {0,0,0};
    unsigned char result[7] = {0x00,0x01};

    if ((0x05 != digit_cnt) && (0x06 != digit_cnt) && (0x04 != digit_cnt)&&(0x07 != digit_cnt))
        return;
    /*buff[0] 存放的是第0位数字，发送的时使用小段发送*/
    if(device_is_last_fractional())
    {
        buff[0] = BIN2BCD(digit[4]*10);
        buff[1] = BIN2BCD(digit[2]*10+ digit[3]);
        buff[2] = BIN2BCD(digit[0]*10 + digit[1]);
        buff[3] = BIN2BCD(0);
        buff[4] = UNIT_188_M3;
        buff[5] = BIN2BCD(digit[4]*10);
        buff[6] = BIN2BCD(digit[2]*10+ digit[3]);
        buff[7] = BIN2BCD(digit[0]*10 + digit[1]);
        buff[8] = BIN2BCD(0);
        buff[9] = UNIT_188_M3;
    }
    else
    {
#if CONFIG_SEVEN_DIGITS
        if(5 == devinfo.digit_count)
        {
            memcpy(result+2,digit,devinfo.digit_count);
            digit_cnt = 7;
            buff[0] = 0;//
            buff[1] = BIN2BCD(result[digit_cnt-2]*10 + result[digit_cnt-1]);
            buff[2] = BIN2BCD(result[digit_cnt-4]*10 + result[digit_cnt-3]);
            buff[3] = BIN2BCD(result[digit_cnt-6]*10 + result[digit_cnt-5]);
            buff[4] = BIN2BCD(result[0]);
            buff[5] = UNIT_188_M3;
            buff[6] = buff[0];
            buff[7] = buff[1];
            buff[8] = buff[2];
            buff[9] = buff[3];
            buff[10] = UNIT_188_M3;
        }
#else
        buff[0] = 0;//
        buff[1] = BIN2BCD(digit[digit_cnt-2]*10 + digit[digit_cnt-1]);
        buff[2] = BIN2BCD(digit[digit_cnt-4]*10+ digit[digit_cnt-3]);
        if(digit_cnt == 0x06)
            buff[3] = BIN2BCD(digit[0]*10 + digit[1]);
        if(digit_cnt == 0x05)
            buff[3] = BIN2BCD(digit[0]);
        if(digit_cnt == 0x04)
            buff[3] = 0;
        buff[4] = UNIT_188_M3;
        buff[5] = buff[0];
        buff[6] = buff[1];
        buff[7] = buff[2];
        buff[8] = buff[3];
        buff[9] = UNIT_188_M3;
#endif
    }
    store_performance();
    send_response_188_frame(CMD_188_READ,frame_did, sizeof(buff),buff);
}

int send_raw_reconginze_number(unsigned char buff[])
{
    send_response_188_frame(CMD_188_READ,frame_did, (devinfo.digit_count)*6,buff);
    return 0;
}

