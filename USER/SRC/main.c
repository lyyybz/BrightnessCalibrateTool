#include <stm32f10x.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "delay.h"
#include "config.h"
#include "snaphist.h"
#include "uart_fifo.h"
#include "i2c.h"
#include "usart.h"
#include "uart.h"
#include "bmp_ops.h"
#include "recognize.h"
#include "bmp.h"
#include "main.h"
#include "dbug.h"
#include "cmd.h"
#include "camera_dev.h"
#include "camera.h"
#include "hardware_layer.h"
#include "protcl.h"
#include "comfunc.h"
#include "protcl.h"
#include "bmp_layer.h"
#include "device_info.h"
#include "motor.h"
#include "proto_camera.h"
#include "hash.h"
#include "mem_manage.h"
#include "JPGENcode.h"
#include "update.h"
#include "main.h"
#include "decode_ebm.h"
#include "task_handle.h"
#include "protcl_7e.h"

#if CONFIG_DEBUG
int motor_direction = MOTOR_STOP;
#endif

int brights[0x100];
int get_brights(void)
{
    uint8_t x;
    int i;
    __IO uint32_t *GPIO_IDR = &(GPIOA->IDR);
    vsync_interrupt_config(0);
    memset(brights,0,sizeof(brights));
    camera_start_capture_rect(0, 320, 0, 240);
	
    EXTI_ClearITPendingBit(EXTI_Line13);
    while(EXTI_GetFlagStatus(EXTI_Line13) == RESET);
	
    for( i = 0 ; i < 320*240 ; i++)
    {
        while( *GPIO_IDR & GPIO_Pin_12);
        while( (*GPIO_IDR & GPIO_Pin_12) == 0);
        x = *GPIO_IDR;
        brights[x]++;
    }
		int bright[0x100];
		memcpy(bright, brights, 1024);
}

static void init_relay_output(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	/* Configure pins as AF pushpull */
  GPIO_ResetBits(GPIOA, GPIO_Pin_15);
  
	GPIO_InitStructure.GPIO_Pin =   GPIO_Pin_4;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOB, &GPIO_InitStructure);
	  
}
void toggle_relay(int k)
{ 
	 while(k--)
	 {
		  GPIO_SetBits(GPIOB, GPIO_Pin_4);
		  GPIO_ResetBits(GPIOB, GPIO_Pin_4);
	 } 
 }

static void SPI1_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    /* Connect pin to Periph */
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE);
		//  GPIO_PinRemapConfig(GPIO_Remap_SPI1,ENABLE);
 
    /* Configure pins as AF pushpull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |  GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

static void SPI1_Initx(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    SPI_InitTypeDef  SPI_InitStructure;

    /* Connect pin to Periph */
		GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST,ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SPI1,ENABLE);
 
		SPI_I2S_DeInit(SPI1);

    /* Configure pins as AF pushpull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 |  GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    //32M / 8 = 4M, ee max speed is 10M when volt is between 2.5 and 4.5
    /* SPI1 configuration */
    SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
    SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
    SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
    SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;
    SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
    SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
    SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_32;
    SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
    SPI_InitStructure.SPI_CRCPolynomial = 7;
    SPI_Init(SPI1, &SPI_InitStructure);
 
    SPI_Cmd(SPI1, ENABLE);
}
 
void SPI1_SendByte(uint8_t byte)
{
  int k = 8; 
  while(k--)
	{		
		GPIOB->BRR = GPIO_Pin_3;
   if(byte &0x80)		 
  {
    GPIOB->BSRR = GPIO_Pin_5;
  }
  else
  {
    GPIOB->BRR = GPIO_Pin_5;
  }
	 byte<<=1;
	 GPIOB->BSRR = GPIO_Pin_3;
  }
	GPIOB->BRR = GPIO_Pin_3;
    /* Wait to receive a byte */
   // while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
   // return SPI_I2S_ReceiveData(SPI1);
   
}

void SPI1_SendByteX(uint8_t byte)
{
    /* Send byte through the SPI1 peripheral */
    SPI_I2S_SendData(SPI1, byte);

    /* Wait to receive a byte */
   // while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_RXNE) == RESET);
    /* Return the byte read from the SPI bus */
   // return SPI_I2S_ReceiveData(SPI1);
}
void testspi(void)
{
  while(1)
  {
	  int i;
		toggle_relay(1);
		for(i = 0 ; i <0x100  ;i++)
		  SPI1_SendByte(i); 
		delay_us(1);
	}
}

void start_camera(void)
{
		set_cmos_pwdn(CMOS_ENABLE);
	  TIM_Cmd(TIM1, DISABLE);
    DMA_Cmd(DMA1_Channel5, DISABLE);
		change_xclk_prescaler(3);
    mycmos_init();
    //comos_0308_write_windows_rect(0, 0, 240, 320);
		//comos_7670_write_windows_rect(0, 0, 240, 320);
}
int main(void)
{
    log_msg("============= app start =============\n");
	
    __disable_irq();

#if CONFIG_BOOT
    NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x2000);//program shifting 0x2000
#endif

    PWR_Configuration();
    hardware_init_high_power();
    camera_init_fifo();
    usart1_nvic_config(1);   
    //watchdog_init(); // 开门狗打开后，led和relay状态跳变
	
    __enable_irq();

    //SPI1_Init();
	 	//init_relay_output();
		start_camera();
		delay_ms(1000);
		//testspi(); 
		
    log_msg("============= init succ =============\n");

    while(1)
    {  
				//capture_bmp_info();
				main_event_handle();
    }
}
