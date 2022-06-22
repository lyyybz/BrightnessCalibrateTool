#include "device_info.h"
#include "uart_fifo.h"
#include "task_handle.h"
#include "main.h"
#include "update.h"
#include "protcl.h"
#include "protcl_7e.h"
#include "hardware_layer.h"
#include "proto_camera.h"
#include "JPGENcode.h"
#include "hash.h"
#include "recognize_cnn.h"
#include "recognize.h"
#include "dbug.h"
#include "cmd.h"
#include "motor.h"

volatile unsigned int  sys_tick_cnt = 0;

extern int  (*capture_bmp)(void);
extern void (*read_bmp)(unsigned char buff[],unsigned char id);

unsigned char raw_number[MAX_DIGIT_CNT*2*3];
void do_task(int task)
{
    hardware_init_high_power();
    delay_ms(40);//等待mbus主机电流基准上升，否则容易产生乱码

    switch(task)
    { 
/*
    case TASK_GET_RAW_IMG_BMP:
        capture_bmp_to_flash(devinfo.crop_rect.left,
                             devinfo.crop_rect.right,
                             devinfo.crop_rect.top,
                             devinfo.crop_rect.bottom);
        cmd = IMG_CMD_RAW_BMP;
        height =  devinfo.crop_rect.bottom - devinfo.crop_rect.top;
        width = devinfo.crop_rect.right - devinfo.crop_rect.left;
        break;
 */

    }  
}

void main_event_handle(void)
{
    flash_led_stop();
    set_relay(0);
    set_led1(0);
		time_count_stop();
    while (check_read());
    // 被测设备上电
    log_msg("============= start work =============\n");
    set_relay(1);
    set_led1(1);
    // 设置本地载波芯片地址
    local_setting();
		time_count_start();
	
    while (!check_read())
    {
        // 接收上位机报文
        scan_soft_uart_channel();
        // 接收串口报文
        scan_plc_channel();
    }
	  log_msg("============= stop work =============\n");
}
