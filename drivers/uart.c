#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stm32f10x.h>
#include "uart.h" 
#include "usart.h" 
void board_putchar(char ch)
{
	USART1_Transmit(ch);
}
int is_board_get_char(void)
{
	return(SET ==  USART_GetFlagStatus(USART1 , USART_FLAG_RXNE));
}
char board_getchar(void)
{
	while (RESET ==  USART_GetFlagStatus(USART1 , USART_FLAG_RXNE));
	return(USART_ReceiveData(USART1));
}

int board_getchar_timeout(char *c, int timeout)
{
	while (timeout-- > 0)
	{
		if (SET ==  USART_GetFlagStatus(USART1 , USART_FLAG_RXNE))
		{
			*c = USART_ReceiveData(USART1);
			return 0;
		}
	}
	return -1;
}

 
