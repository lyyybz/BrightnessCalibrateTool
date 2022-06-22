/******************** (C) COPYRIGHT 2010 STMicroelectronics ********************
* File Name          : stm32f10x_it.c
* Author             : MCD Application Team
* Version            : V3.2.1
* Date               : 07/05/2010
* Description        : Main Interrupt Service Routines.
*                      This file provides template for all exceptions handler
*                      and peripherals interrupt service routine.
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stm32f10x_it.h>
#include "Nvic_Exit.h"
#include <usart.h>
#include "recognize.h"
#include "usart.h"
#include "main.h"
#include "uart_fifo.h"
#include "camera.h"
#include "hardware_layer.h"
#include "usart.h"
#include "config.h"
#include "protcl_7e.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/*******************************************************************************
* Function Name  : NMI_Handler
* Description    : This function handles NMI exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void NMI_Handler(void)
{
    while(1);
}

/*******************************************************************************
* Function Name  : HardFault_Handler
* Description    : This function handles Hard Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void HardFault_Handler(void)
{
    /* Go to infinite loop when Hard Fault exception occurs */
    while (1)
    {
    }
}

/*******************************************************************************
* Function Name  : MemManage_Handler
* Description    : This function handles Memory Manage exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void MemManage_Handler(void)
{
    /* Go to infinite loop when Memory Manage exception occurs */
    while (1)
    {
    }
}

/*******************************************************************************
* Function Name  : DMA1_Channel2_IRQHandler
* Description    : This function handles DMA1 Channel 2 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel2_IRQHandler(void)
{
    while(1);
}

void DMA1_Channel5_IRQHandler(void)
{
    if(
        //   (DMA_GetITStatus(DMA1_IT_HT5) != RESET)
        //  ||
        (DMA_GetITStatus(DMA1_IT_TC5) != RESET)
    )
    {
        DMA_ClearITPendingBit(DMA1_IT_TC5);
        //got a line of pixels,so
        fill_line_to_itfifo( );

    }
    else
    {
        //unexpected error!!!
        DMA1->IFCR = DMA1->ISR;
    }
}
/*******************************************************************************
* Function Name  : BusFault_Handler
* Description    : This function handles Bus Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void BusFault_Handler(void)
{
    /* Go to infinite loop when Bus Fault exception occurs */
    while (1)
    {
    }
}

/*******************************************************************************
* Function Name  : UsageFault_Handler
* Description    : This function handles Usage Fault exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void UsageFault_Handler(void)
{
    /* Go to infinite loop when Usage Fault exception occurs */
    while (1)
    {
    }
}

/*******************************************************************************
* Function Name  : SVC_Handler
* Description    : This function handles SVCall exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SVC_Handler(void)
{
}

/*******************************************************************************
* Function Name  : DebugMon_Handler
* Description    : This function handles Debug Monitor exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DebugMon_Handler(void)
{
}

/*******************************************************************************
* Function Name  : PendSV_Handler
* Description    : This function handles PendSVC exception.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void PendSV_Handler(void)
{
}

/*******************************************************************************
* Function Name  : SysTick_Handler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
extern volatile unsigned int  sys_tick_cnt;

void SysTick_Handler(void)
{
    sys_tick_cnt++;
    watchdog_50ms_tick();
    uart_tick_hook(0);
    flash_led();
		time_count();
		plc_time_count();
}

/******************************************************************************/
/*            STM32F10x Peripherals Interrupt Handlers                        */
/******************************************************************************/

#ifndef STM32F10X_CL
/*******************************************************************************
* Function Name  : USB_HP_CAN1_TX_IRQHandler
* Description    : This function handles USB High Priority or CAN TX interrupts requests
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_HP_CAN1_TX_IRQHandler(void)
{
    //CTR_HP();
}

/*******************************************************************************
* Function Name  : USB_LP_CAN1_RX0_IRQHandler
* Description    : This function handles USB Low Priority or CAN RX0 interrupts
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    //USB_Istr();
}
#endif /* STM32F10X_CL */

#if defined(STM32F10X_HD) || defined(STM32F10X_XL)
/*******************************************************************************
* Function Name  : SDIO_IRQHandler
* Description    : This function handles SDIO global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SDIO_IRQHandler(void)
{
    /* Process All SDIO Interrupt Sources */
    //SD_ProcessIRQSrc();

}
#endif /* STM32F10X_HD | STM32F10X_XL*/

#ifdef STM32F10X_CL
/*******************************************************************************
* Function Name  : OTG_FS_IRQHandler
* Description    : This function handles USB-On-The-Go FS global interrupt request.
*                  requests.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void OTG_FS_IRQHandler(void)
{
    STM32_PCD_OTG_ISR_Handler();
}
#endif /* STM32F10X_CL */

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/
volatile int send_enable =0;
volatile int Vsync_Flag=0;
/*******************************************************************************
* Function Name  : USART1_IRQHandler
* Description    : This function handles USART1 global interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int count = 0;
uint8 rxbuffer[0x100];
void USART1_IRQHandler(void)
{
    unsigned char c=0x55;
    int k;

    if(USART_GetFlagStatus(USART1,USART_FLAG_ORE ) != RESET )
    {
        //over run
        USART_ClearFlag(USART1,USART_FLAG_ORE);
    }

    if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
    {
        /* Read one byte from the receive data register */
        c = USART_ReceiveData(USART1);
        uart_chn_rx_bytes(0,&c,1);
        uart_rx_hook(0x00);
        rxbuffer[count++] = c;
				if(count >=13)
				{
					count = 0;
				}
    }
#if 1
    if (USART_GetITStatus(USART1, USART_IT_TXE) != RESET)
    {
        k = uart_chn_tx_bytes(0x00, &c, 1);
        if (k > 0x00)
        {
            /* Write one byte to the transmit data register */
            USART_SendData(USART1, c);
            uart_tx_hook(0x00);
        }
        else
        {
            /* Disable the USART1 Transmit interrupt */
            USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

        }
    }
#endif

}

void USART_COM2_IRQHandler(void)
{
}
void toggle_relay(int k);
void EXTI15_10_IRQHandler(void)
{
    int i = EXTI->PR;;
    //do some init job,start dma?
    if(EXTI_GetITStatus(EXTI_Line13) != RESET)
    {
        /* Clear the EXTI line 13 pending bit */
        EXTI_ClearITPendingBit(EXTI_Line13);

			  //toggle_relay(2);
			
        //if(START_CAP == itfifo.cap_state)
        {
            itfifo.cap_state = CAPTURING;
            DMA_ITConfig(DMA1_Channel5,DMA_IT_TC,DISABLE);
            camera_frame_begin_callback();
            DMA_ClearFlag(DMA1_FLAG_TC5);
            DMA_ITConfig(DMA1_Channel5,DMA_IT_TC,ENABLE);
        }
				/*
        else if (CAP_REQUEST_FINISHING == itfifo.cap_state)
        {
            DMA_ITConfig(DMA1_Channel5, DMA_IT_TC, DISABLE);
            camera_frame_end_callback();
            vsync_interrupt_config(0);
            itfifo.cap_state = CAP_FINISHED;
        }
				*/
				//toggle_relay(4);
        itfifo.camera_scan_line = 0;

    }
    else
    {
        i++;
    }
}

extern volatile struct soft_uart_chn _uart_chns[1];
void TIM_OC2Init2(TIM_TypeDef* TIMx, uint16 capture)
{
    TIM_OCInitTypeDef TIM_OCInitStructure;
    TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_Timing;
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High;
    TIM_OCInitStructure.TIM_Pulse = capture;
    TIM_OC2Init(TIMx, &TIM_OCInitStructure);
}

void EXTI9_5_IRQHandler(void) 
{ 
	if(EXTI_GetITStatus(EXTI_Line5) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line5);
		
		EXTI_State_Change(DISABLE);
		
		uint16_t capture = TIM_GetCounter(TIM2);
		capture += BitInterval >> 2;
    capture += BitInterval;
		
		// clear
		struct soft_uart_chn* pchn = &_uart_chns[0];
		pchn->rx.bitcnt = 1;
		pchn->rx.data = 0x00;
		pchn->rx.bit = 0x01;
		
		TIM_OC2Init2(TIM2, capture);
		TIM_ClearFlag(TIM2, TIM_IT_CC2);
		TIM_ITConfig(TIM2, TIM_IT_CC2, ENABLE);
	}
}
