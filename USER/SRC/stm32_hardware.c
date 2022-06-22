#include <stdint.h>
#include "bmp_layer.h"
#include "alloter.h"
#include "stm32_hardware.h"
#include "cnndriverpc.h"
#include "device_info.h"
//#include "cnnfeature.h"

#if 1
void Get_ChipID(uint32_t *ChipID)
{
    ChipID[0] = 0x01020304;
    ChipID[1] = 0x05060708;
    ChipID[2] = 0x09101112;
}
void hardware_init_high_power(void)
{

}

void hardware_init_low_power(void)
{

}

void delay_ms(int ms)
{

}

void usart_init(int baud_rate,unsigned char is_even)
{

}

//int sys_uart_peek_data(int chn,unsigned char buffer[],int len)
//{

//}

//int sys_uart_read(int chn, unsigned char buffer[],int len)
//{

//}

//int sys_uart_sync_write(int chn,unsigned char buffer[],int len)
//{

//}
void USART1_Transmit(unsigned char data)
{

}

int sys_uart_sync_write(int chn,unsigned char buffer[],int len)
{
    int i = 0;
    while(i < len)
    {
        if(0x7E == buffer[i])//7e head
            break;
        i++;
    }
    memset(result_frame, 0x00, 100);
    memcpy(result_frame, &buffer[i], len-i);
    return len;
}



//extern __GET_LAST_BIT_POS(unsigned int x);
OS_CPU_SR OS_CPU_SR_Save(void)
{

}
void OS_CPU_SR_Restore(OS_CPU_SR cpu_sr)
{

}

void feed_dog()
{

}
void watchdog_set_free_feed_time(int second)
{

}
void watchdog_clr_free_feed_time()
{

}

update_1s_tick()
{

}
void setlight(int x)
{

}
void set_cmos_pwdn(int lvl)
{

}
mycmos_init()
{

}
wfi()
{

}

int get_digits_from_camera_fast(void)
{
    //unsigned char digit_line[MAX_DIGIT_CNT*DIGIT_WIDTH]={0};
    //dataflash_write(RECGONIZE_IMAGE_ADDR,digit_line,DIGIT_WIDTH*(devinfo.water_meter_digit_numb));
}

int  get_line_from_flash(int y,unsigned char buff[])
{

}

int handle_update_task(void)
{

}

int is_in_updateing(void)
{
    return 0;
}


void camera_frame_end_callback(void)
{

}

void read_digit_from_flash(unsigned char buff[],unsigned char id)
{
    //CNNFeature *feature = CNNFeature::sharedCNNFeature();
    //feature->getBmp(buff);
    read_sample_from_flash(buff,0);
}
int is_dataflash_id_correct()
{
    return 1;
}
#endif
