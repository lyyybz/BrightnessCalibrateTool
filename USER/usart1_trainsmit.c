#include "stm32f10x.h"	 
//#include "my_type.h"
#include "usart.h"
#include "fifo.h"
#include "hardware_layer.h"
#include "device_info.h"
#include "config.h"
#include "usart.h"
#include "config.h"

void USART1_Transmit(unsigned char  data)
{
    /*must wait uart send complete*/
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET)
    {    
    } 

    USART_SendData(USART1, data); 

    /*must wait uart send complete*/
    while (USART_GetFlagStatus(USART1, USART_FLAG_TC) == RESET) 
    { 
       //light_sync_status();
    } 
}

