
#include "stm32f10x.h"
//#include "my_type.h"
#include "usart.h"
#include "fifo.h"
#include "hardware_layer.h"
#include "device_info.h"
#include "protcl.h"

void usart1_nvic_config(int state)
{
    NVIC_InitTypeDef NVIC_InitStructure;

    /* USART1 IRQ Channel configuration */
    NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x03;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;
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



void usart_init(int baud_rate,unsigned char is_even)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;


    /* USART1 Pins configuration ************************************************/
    /* Connect pin to Periph */
    /* Configure USART1 Tx (PA.09) as alternate function push-pull */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    //GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    /* Configure USART1 Rx (PA.10) as input floating */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = baud_rate;

    if(is_even)
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_9b;
        USART_InitStructure.USART_Parity = USART_Parity_Even;
    }
    else
    {
        USART_InitStructure.USART_WordLength = USART_WordLength_8b;
        USART_InitStructure.USART_Parity = USART_Parity_No;
    }

    USART_InitStructure.USART_StopBits = USART_StopBits_1;

    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

    USART_Init(USART1, &USART_InitStructure);

    /* Enable 8xUSARTs Receive interrupts */
    USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
    USART_ITConfig(USART1, USART_IT_TXE, DISABLE);

    /* Enable the 8xUSARTs */
    USART_Cmd(USART1, ENABLE);

}

void UART_IT_Ctrl(int chn,FunctionalState TxItNewState,FunctionalState RxItNewState)
{
    switch(chn)
    {
    case 0x00:
        USART_ITConfig(USART1, USART_IT_RXNE, RxItNewState);
        USART_ITConfig(USART1, USART_IT_TXE, TxItNewState);
        break;

    default:

        break;  //error chn

    }
}

int sys_uart_sync_write(int chn,unsigned char buffer[],int len)
{
    int i=0;
    for(i =0 ; i < len; i++)
    {
        USART1_Transmit(buffer[i]);
    }
    return len;
}
