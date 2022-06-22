#ifndef _DEV_FIFO_H_
#define _DEV_FIFO_H_

#ifdef __cplusplus
 extern "C" {
#endif

#include  "alloter.h"

#define EEPROM_CFG    0xAA
#define FRAM_CFG      0x55

#define		PARITY_NO		0x01
#define		PARITY_ODD		0x02
#define		PARITY_EVEN		0x04
struct _UART_CFG
{
	unsigned int baud;
	unsigned char byte_bit;
	unsigned char stop_bit;
	unsigned char parity;
};

struct _UART_FILE_INFOR
{
	struct _CHN_SLOT    tx_slot;
	struct _CHN_SLOT    rx_slot;
	unsigned char 	busy_rxing;
	unsigned char 	over_time_tick;
	unsigned char 	txing_no_rx;
};

 
void uart_tick_hook(int chn);
void uart_tx_hook(int chn);
void uart_rx_hook(int chn);
void alarm_uart_rx_finish_flag(int chn);
int uart_chn_tx_bytes(int chn,unsigned char buffer[] ,int len);
int uart_chn_rx_bytes(int chn,unsigned char buffer[] ,int len);
int sys_uart_read(int chn, unsigned char buffer[],int len);
int sys_uart_peek_data(int chn,unsigned char buffer[],int len);
void uart_chn_init(int chn); 
void hard_reset_now(void); 
int sys_uart_get(int chn, unsigned char buffer[],int len);
int uart_get_received_len(int chn);

#if !defined(KEIL)
int sys_uart_write(int chn,unsigned char buffer[],int len);
#endif

void test_uart_send(void);

#ifdef __cplusplus
}
#endif

#endif

