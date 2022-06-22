#ifndef _UART_H_
#define _UART_H_

#include <stdio.h>
#include <stdint.h>
 

void init_uart(void);
void board_putchar(char ch);
char board_getchar(void);

  int is_board_get_char(void);
 

int board_getchar_timeout(char *c, int timeout);
#endif
