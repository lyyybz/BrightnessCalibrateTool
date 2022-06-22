#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>
#include "config.h"
#include "camera.h"
#include "recognize.h"
#include "hardware_layer.h"
#include "main.h"
#include "usart.h"
#include "camera_dev.h"
#include "comfunc.h"
#include "protcl.h"
#include "delay.h"
#include "cache_control.h"
#include "math.h"
#include "bmp_layer.h"
#include "device_info.h"
#include "protcl_7e.h"

struct IT_FIFO itfifo;
#define disable_interrupt()	__SETPRIMASK()
#define enable_interrupt()	__RESETPRIMASK()

#define VSYNC_EXIT_LINE       EXTI_Line13


volatile unsigned short SRC_Buffer[DMA_BUF_SZ];

static int get_data_sz(int rx,int tx,int sz)
{
    if (rx >= tx)
    {
        return(rx - tx);
    }
    else
    {
        return(rx+ sz - tx);
    }
}

static int get_free_sz(int rx,int tx,int sz)
{
    return(sz - 1 - get_data_sz(rx,tx,sz));
}

//如果丢一帧，便会将本帧剩余的线都扔掉。从下一帧相同的位置继续取数据。
void fill_line_to_itfifo(void)
{
    unsigned int i, px, left, right;

    /*缓存足够，并且图像连续*/
    if ((itfifo.camera_scan_line == itfifo.rect.top)
            && (itfifo.rect.top < itfifo.rect.bottom)
            && (get_free_sz(itfifo.rx, itfifo.tx, itfifo.sz) >= (itfifo.rect.right - itfifo.rect.left)))
    {
			//log_msg("fill, itfifo.rect.top: %d\n", itfifo.rect.top);
        left = itfifo.rect.left;
        right = itfifo.rect.right;

#if ONLY_MONO_CHANEL
        //that what we want,so
        for (i = left; i < right; i+=2)
        {
            px = SRC_Buffer[i/2];

            itfifo.buffer[itfifo.rx++] = px ;
            if (itfifo.rx >= itfifo.sz)
            {
                itfifo.rx = 0;
            }
            px >>= 8;
            itfifo.buffer[itfifo.rx++] = px ;
            if (itfifo.rx >= itfifo.sz)
            {
                itfifo.rx = 0;
            }
        }
				if(itfifo.rect.top == 57){
					//log_frame2("", (unsigned char*)&itfifo.buffer, 2880);
				}
#else
        for (i = left; i < right; i++)
        {
            px = SRC_Buffer[i];
            px = (px>>8) + ((px&0xff)<<8);
            px >>= 5;
            px &= 0x3f;

            itfifo.buffer[itfifo.rx++] = px << 2;
            if (itfifo.rx >= itfifo.sz)
            {
                itfifo.rx = 0;
            }
        }
#endif
        itfifo.rect.top++;
        if (itfifo.rect.top >= itfifo.rect.bottom)
        {
            itfifo.cap_state = CAP_REQUEST_FINISHING;
        }
    }
    itfifo.timeout_line_cnt = 0;
    itfifo.camera_scan_line++;
}


/*如果没有数据,便会等待数据超时*/
int get_a_line_from_fifo(int y,unsigned char line[])
{
    int k,tx;
    long long  delay =0 ;
    int linelen =  itfifo.rect.right - itfifo.rect.left;

    while(get_data_sz(itfifo.rx,itfifo.tx,itfifo.sz) < linelen )
    {
      
    }
    tx = itfifo.tx;
    k =  itfifo.sz - tx;
		//log_msg("tx: %4d, k: %4d\n", tx, k);
    if (linelen <= k)
    {
        memcpy(&line[0],(unsigned char*)&itfifo.buffer[tx],linelen);
        tx += linelen;
        if (tx >= itfifo.sz) tx -= itfifo.sz;
    }
    else
    {
        memcpy(&line[0],(unsigned char*)&itfifo.buffer[tx],k);
        memcpy(&line[k],(unsigned char*)&itfifo.buffer[0],linelen - k);
        tx = linelen - k;
    }
    itfifo.tx = tx;
    itfifo.timeout_line_cnt = 0;
    return (linelen);
}

static int itfifo_set_buff_and_rect(unsigned char xbuffer[],int xsz,struct TRECT *prect)
{
    vsync_interrupt_config(0);
    itfifo.frame_id = 0;
    itfifo.cap_state = START_CAP;
    itfifo.tx = 0;
    itfifo.rx = 0;
    itfifo.camera_scan_line = 0;
    itfifo.next_store_line = 0;
    itfifo.timeout_line_cnt = 0;
    memcpy((struct TRECT *)&itfifo.rect,prect,sizeof(*prect));
    itfifo.buffer = xbuffer;
    itfifo.sz = xsz;
    EXTI_ClearITPendingBit(VSYNC_EXIT_LINE);
    vsync_interrupt_config(1);
    return(1);
}

short it_fifo_get_width()
{
    return itfifo.rect.right - itfifo.rect.left;
}


void camera_init_fifo()
{
    itfifo.cap_state = CAP_FINISHED;
}


void vsync_interrupt_config(int state)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* Enable and set EXTI13 Interrupt */
    NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;

    if (state)
    {
        NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    }
    else
    {

        NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
    }

    NVIC_Init(&NVIC_InitStructure);
}

void camera_frame_begin_callback(void)
{
   // change_xclk_prescaler(2);
    //SetSysClock_max108();
    TIM_Cmd(TIM1, DISABLE);
    TIM_SetCounter(TIM1, 0);
    TIM_ClearFlag(TIM1, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4);

    TIM_ClearFlag(TIM1, TIM_IT_Update | TIM_IT_CC1 | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4);
    DMA_Cmd(DMA1_Channel5, DISABLE);
    camera_read_dma_reinit();
    DMA_Cmd(DMA1_Channel5, ENABLE);

    TIM_Cmd(TIM1, ENABLE);
}

void camera_frame_end_callback(void)
{
    //TIM_Cmd(TIM1, DISABLE);
    //DMA_Cmd(DMA1_Channel5, DISABLE);
}

static uint8_t data_xxx[CAMERA_FIFO_SIZE];
void camera_start_capture_rect(int left,int right,int top,int bottom)
{
    struct TRECT rect; 
    //set_cmos_pwdn(CMOS_ENABLE);
	  
	  TIM_Cmd(TIM1, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);
	
    //mycmos_init();
    //comos_0308_write_windows_rect(left, top, bottom-top, right-left);

    rect.left = 0;
    rect.right = right-left;
    rect.top = 0;
    rect.bottom = bottom-top;

     
    itfifo_set_buff_and_rect(data_xxx,CAMERA_FIFO_SIZE,&rect);
}

 

